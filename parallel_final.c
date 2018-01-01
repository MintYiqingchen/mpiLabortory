#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "mpi.h"
int* local_str2int(char* argv[], int lo, int localsize) {
	int* array = (int*)malloc(localsize * sizeof(int));
	int i;
	for (i = 0; i < localsize; i++) {
		array[i] = atoi(argv[lo + i]);
	}
	return array;
}

int partition(int* array, int len) {
	int pivot = array[0], lo = 0, hi = len - 1;
	while (lo < hi) {
		while (lo < hi && array[hi] > pivot) hi--;
		while (lo < hi && array[lo] <= pivot) lo++;
		if (lo != hi)
		{
			array[lo] = array[lo] ^ array[hi];
			array[hi] = array[lo] ^ array[hi];
			array[lo] = array[lo] ^ array[hi];
		}
	}
	array[0] = array[lo];
	array[lo] = pivot;
	// int i;
	// for(i=0;i<len;i++)
	// 	printf("array %d\n", array[i]);
	// printf("lo%d\n", lo);
	return lo;
}
void quicksort(int *array, int lo, int hi) {
	int len = hi - lo;
	if (len <= 1) return;

	int *newp = array + lo;
	int pidx = partition(newp, len);
	quicksort(newp, 0, pidx);
	quicksort(newp, pidx + 1, len);
}
void merge(int* array, int* begin1, int* end1, int* begin2, int* end2) {
	/*
	@array: global array
	return: begin1<-new begin, end2<-new end
	*/
	// printf("begin1:%ld end1:%ld begin2:%ld end2:%ld\n", begin1, end1, begin2, end2);
	if (end1 == begin1) return;
	int* frontbuffer = (int*)malloc((end1 - begin1) * sizeof(int)), *currp = begin1;
	int *bufferbegin = frontbuffer, *bufferend = bufferbegin + (end1 - begin1);
	memcpy(bufferbegin, begin1, (end1 - begin1) * sizeof(int));

	// int* i;
	// for (i = begin1; i != end2; ++i)
	// {
	// 	printf("raw array: %d\n", *i);
	// }
	// printf("bufferfront:%ld bufferend:%ld\n", frontbuffer, bufferend);

	while (bufferbegin != bufferend && begin2 != end2) {
		if (*bufferbegin <= *begin2) {
			*currp = *bufferbegin;
			bufferbegin++;
		}
		else {
			*currp = *begin2;
			begin2++;
		}
		// printf("currp:%ld value:%d\n", currp, *currp);
		currp++;
	}
	while (bufferbegin != bufferend) {
		*currp = *bufferbegin;
		bufferbegin++;
		currp++;
	}
	// for (i = begin1; i != end2; ++i)
	// {
	// 	printf("after merge array: %d\n", *i);
	// }
	free(frontbuffer);
}
void _binaryCut(int* array, int** begins, int** ends, int lo, int hi) {
	/*
	@array: global array
	@displs: displs[i] is beginning of i-th section
	@lo: lo-th segment
	@hi: hi-th segment
	@begins @ends: pointer of segment begin and end
	*/
	if (hi - lo <= 0) return;
	int mid = lo + (hi - lo) / 2;
	// printf("lo:%d mid:%d hi:%d\n", lo, mid, hi);

	_binaryCut(array, begins, ends, lo, mid);
	_binaryCut(array, begins, ends, mid + 1, hi);
	merge(array, begins[lo], ends[mid], begins[mid + 1], ends[hi]);

//	int* i;
//	for (i = begins[lo]; i != ends[hi]; ++i)
//	{
//		printf("array: %d\n", *i);
//	}
//	printf("\n");
}
void mul_mergesort(int* array, int* displs, int segnum, int len) {
	/*
	@displs: displs[i] is beginning of i-th section
	@segnum: the number of segments
	@len: length of array
	*/
	int i;

	int** begins = malloc(sizeof(int*)*segnum), **ends = malloc(sizeof(int*)*segnum);
	// int* begins[50], *ends[50];
	for (i = 0; i < segnum; i++) {
		begins[i] = array + displs[i];
		if (i == segnum - 1)
			ends[i] = begins[i] + len - displs[segnum - 1];
		else
			ends[i] = begins[i] + displs[i + 1] - displs[i];
	}
//	for (i = 0; i<segnum; i++)
//		printf("begin %ld end %ld\n", begins[i], ends[i]);

	_binaryCut(array, begins, ends, 0, segnum - 1);

	// free 2-level pointer
	for (i = 0; i < segnum; ++i)
	{
		begins[i]=NULL;
		ends[i]=NULL;
	}
	free(begins); free(ends);
}
void mul_mergesort1(int *array, int seglen, int segnum){
	/*
	@seglen: length of each segment
	@segnum: how many segments are there
	*/
	int* displs = calloc(segnum, sizeof(int));
	int i;
	for(i = 0; i<segnum; ++i){
		displs[i]=seglen*i;
	}
	mul_mergesort(array, displs, segnum, seglen*segnum);
	free(displs);
}
void main(int argc, char* argv[]){
	MPI_Init(&argc, &argv);
	#ifdef MPI_DEBUG
    	int bug_break = 1;
	while(bug_break){
	}
	#endif
    	if (argc<3){
        	MPI_Finalize();
        	return;
    	}

	int psize=0;
	int prank=0;
	MPI_Comm_size(MPI_COMM_WORLD, &psize);
	MPI_Comm_rank(MPI_COMM_WORLD, &prank);
	// generate numbers
	int* localarray;
	time_t start, end;

	// every process get local numbers from argv, [lo, lo+localsize)
	int i, globalsize = atoi(argv[1]);
	int* globalarray = (int*)calloc(globalsize, sizeof(int));
	int* recvbuffer = (int*)calloc(psize, sizeof(int));
	int* sendbuffer = (int*)calloc(psize, sizeof(int));
	int *sdispls=(int*)calloc(psize, sizeof(int)), *rdispls=(int*)calloc(psize, sizeof(int));

	//read number from file
	if(prank==0){
		FILE* fh;
		fh=fopen(argv[2], "r");
		for(i=0; i<globalsize; i++){
			fscanf(fh,"%d", &globalarray[i]);
		}
		fclose(fh);
	}

	int localsize = globalsize/psize, lo = localsize*prank, remain = globalsize-localsize*psize;
	if(prank >= psize - remain){
		localsize++; // final process has one more number
        	lo += remain -psize +prank;
	}
	printf("process %d: localsize %d lo %d\n", prank, localsize, lo);
	MPI_Bcast((void*)globalarray, globalsize, MPI_INT, 0, MPI_COMM_WORLD);
	
	// convert string into integer or get local array from p 0
	// localarray = local_str2int(argv, lo+1, localsize);
	localarray = (int*)malloc(localsize * sizeof(int));
	localarray = memcpy(localarray, globalarray+lo, sizeof(int)*localsize);
	// get current time
	start = time(NULL);

	// local quicksort
	int pivotidx = partition(localarray, localsize);
	quicksort(localarray, 0, pivotidx);
	quicksort(localarray, pivotidx+1, localsize);
//	for (i = 0; i < localsize; i++)
//		printf("after local sort %d: %d\n",prank, localarray[i]);


	// sample p-1 numbers
	int w = (int)(globalsize*1.0/psize/psize);
	for(i = 1; i <= psize-1; i++){
		sendbuffer[i-1] = localarray[i * w];
	}

	// send to p0, receive p-1 sampled elements
	MPI_Status* recvinfo = (MPI_Status*)malloc(sizeof(MPI_Status));
	if(prank != 0){
		MPI_Send((void*)sendbuffer, psize-1, MPI_INT, 0, prank, MPI_COMM_WORLD);
	}else{
		// receive all sample pivots
		int samplelength = (psize-1)*psize;
		MPI_Request* requests = (MPI_Request*)malloc((psize-1)*sizeof(MPI_Request));

		recvbuffer = (int*)realloc(recvbuffer, samplelength*sizeof(int));
		memcpy(recvbuffer, sendbuffer, (psize-1)*sizeof(int));
		for(i = 1; i < psize; i++){
			MPI_Irecv(recvbuffer + (psize-1)*i, psize-1, MPI_INT, i, i, MPI_COMM_WORLD,
				&requests[i-1]);
		}
		// wait all non-blocking
		for(i = 1; i < psize; i++){
			MPI_Wait(&requests[i-1], recvinfo);
//			printf("sampled elements from process %d received.\n", i);
		}

		// local merge sort
		mul_mergesort1(recvbuffer, psize-1, psize);
//		for (i = 0; i < psize*(psize-1); i++)
//			printf("mul merge samples %d: %d\n",prank, recvbuffer[i]);

		// choose p-1, send to every process
		for(i = 1; i <= psize-1; i++){
			sendbuffer[i-1] = recvbuffer[i * (psize-1)];
			printf("choosed sample: %d\n", sendbuffer[i-1]);
		}
	}
	MPI_Bcast((void*)sendbuffer, psize-1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);

	// partition local sorted array
	int *section_length = (int*)calloc(psize, sizeof(int));
	int count=0, j;
	remain = localsize;
	for(j = 0, i = 0; j < psize-1; j++){
		count = 0;
		while(i < localsize && localarray[i] <= sendbuffer[j]){
			count++;
			i++;
		}
		section_length[j] = count;
		remain -= count;
	}
	section_length[psize-1] = remain;
//	for (i = 0; i < psize; i++)
//		printf("process %d segment length: %d\n",prank, section_length[i]);


	// send i-th section to i-th process
	MPI_Alltoall((void*)section_length, 1, MPI_INT, (void*)recvbuffer, 1, MPI_INT, MPI_COMM_WORLD);
//	for (i = 0; i < psize; i++)
//		printf("process %d recv num: %d\n",prank, recvbuffer[i]);

	localsize = 0;
	sdispls[0] = 0; rdispls[0] = 0;
	for(i = 1; i < psize; i++){
		sdispls[i] = sdispls[i-1]+section_length[i-1];
		rdispls[i] = rdispls[i-1]+recvbuffer[i-1];
//		printf("process %d send start: %d send length:%d\n",prank, sdispls[i], section_length[i]);
	}
	localsize = recvbuffer[psize-1] + rdispls[psize-1];
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Alltoallv((void*)localarray, section_length, sdispls, MPI_INT,
		(void*)globalarray, recvbuffer, rdispls, MPI_INT, MPI_COMM_WORLD);
	
//	for (i = 0; i < localsize; i++)
//		printf("process %d partition before merge: %d\n",prank, globalarray[i]);

	MPI_Allgather(&localsize, 1, MPI_INT, recvbuffer, 1, MPI_INT, MPI_COMM_WORLD); //gather local size from ALL processes


	// mergesort
	mul_mergesort(globalarray, rdispls, psize, localsize);
	MPI_Barrier(MPI_COMM_WORLD);
//	for (i = 0; i < localsize; i++)
//		printf("process %d partition after merge: %d\n",prank, globalarray[i]);


	// gather sorted array
	rdispls[0] = 0;
	for(i = 1; i < psize; i++)
		rdispls[i]=rdispls[i-1]+recvbuffer[i-1];

	MPI_Allgatherv((void*)globalarray, localsize, MPI_INT,
		(void*)globalarray, recvbuffer, rdispls, MPI_INT, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);

	// output
	if(prank==0){
		end = time(NULL);
		printf("The difference is: %f seconds\n",difftime(start,end));
/*		FILE* f = fopen("/home/mintyi/codework/mpiLabortory/output.txt", "w");
		for(i = 0; i < globalsize; i++){
			fprintf(f, "%d, ", globalarray[i]);
		}
		fprintf(f, "\n");
		fclose(f);*/
		for (i = 0; i<globalsize-1; i++){
			if(globalarray[i]>globalarray[i+1]){
				printf("didnt pass test on res[%d]:%d res[%d]:%d\n", i,globalarray[i],i+1,globalarray[i+1]);
			}
		}
		printf("test end\n");
	}

	// free memory
	free(localarray);
 	free(sendbuffer);
	free(recvbuffer);
	free(section_length);
	free(sdispls); free(rdispls);

	MPI_Finalize();
}
