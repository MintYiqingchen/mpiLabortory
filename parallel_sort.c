#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include "mpi.h"

void main(int argc, char* argv[]){
	MPI_Init(&argc, &argv);
	int psize=0;
	int prank=0;
	MPI_Comm_size(MPI_COMM_WORLD, &psize);
	MPI_Comm_rand(MPI_COMM_WORLD, &prank);
	// generate numbers
	int* localarray;
		
	// every process get local numbers from argv, [lo, lo+localsize)
	int i, globalsize = argc-1;
	int localsize = globalsize/psize, lo = localsize*prank;
	if(prank==psize-1 && localsize*p < globalsize){
		localsize++; // final process has one more number
	}
	printf("process %d: localsize %d lo %d\n", prank, localsize, lo);

	// convert string into integer
	localarray = local_str2int(argv, lo, localsize);

	// local quicksort
	int pivotidx = partition(localarray, localsize);
	quicksort(localarray, lo, pivotidx);
	quicksort(localarray, pivotidx+1, lo+localsize);

	// sample p-1 numbers
	int w = (int)(globalsize*1.0/psize/psize);
	int* sendbuffer = (int*)calloc(psize, sizeof(int));
	for(i = 1; i <= psize-1; i++){
		sendbuffer[i-1] = localarray[i * w];
	}

	// send to p0, receive p-1 sampled elements
	int* recvbuffer = (int*)calloc(psize, sizeof(int));
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
			MPI_IRecv(recvbuffer + (psize-1)*i, psize-1, MPI_INT, i, i, MPI_COMM_WORLD,
				&requests[i-1]);
		}
		// wait all non-blocking
		for(i = 1; i < psize; i++){
			MPI_Wait(&requests[i-1], recvinfo);
			printf("sampled elements from process %d received.\n", i);
		}

		// local merge sort
		mul_mergesort1(recvbuffer, psize-1, psize);

		// choose p-1, send to every process
		for(i = 1; i <= psize-1; i++){
			sendbuffer[i-1] = localarray[i * (psize-1)];
		}
	}
	MPI_Bcast((void*)sendbuffer, psize-1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);

	// partition local sorted array
	int *section_length = (int*)calloc(psize, sizeof(int));
	int count=0, j;
	for(i = 0, j = 0; i < localsize; i++){
		if(localarray[i] > sendbuffer[j]){
			section_length[j] = count;
			count = 0;
			j++;
		}
		else if(i==localsize-1)
			section_length[j] = ++count;
		else count++;
	}

	// send i-th section to i-th process
	int* globalarray = (int*)calloc(globalsize, sizeof(int));
	MPI_Alltoall((void*)section_length, 1, MPI_INT, (void*)recvbuffer, 1, MPI_INT, MPI_COMM_WORLD);
	int sdispls[psize], rdispls[psize]; 
	memset(sdispls, 0, sizeof(sdispls)); memset(rdispls, 0, sizeof(rdispls));
	localsize = 0;

	for(i = 1; i < psize; i++){
		sdispls[i] = sdispls[i-1]+section_length[i-1];
		rdispls[i] = rdispls[i-1]+recvbuffer[i-1];
		localsize += rdispls[i];
	}
	MPI_Alltoallv((void*)localarray, section_length, sdispls, MPI_INT,
		(void*)globalarray, recvbuffer, rdispls, MPI_INT, MPI_COMM_WORLD);
	MPI_Gather(&localsize, 1, MPI_INT, recvbuffer, 1, MPI_INT, 0, MPI_COMM_WORLD); //0 gather size from processes

	// mergesort
	mul_mergesort(globalarray, rdispls, psize, globalsize);
	MPI_Barrier(MPI_COMM_WORLD);

	// gather sorted array
	if(prank==0){
		for(i = 1; i < psize; i++)
			rdispls[i]=rdispls[i-1]+recvbuffer[i-1];
	}
	MPI_Allgatherv((void*)globalarray, localsize, MPI_INT, 
		(void*)globalarray, recvbuffer, rdispls, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);
	
	// output
	if(prank==0){
		FILE* f = fopen("test.txt", "w");
		for(i = 0, count = 0; i < psize; i++){
			for(j = 0; j<recvbuffer[i]; j++){
				fprintf(f, "%d, ", globalarray[count]);
				count++;
			}
		}
		fclose(f);
	}

	// free memory
	free(localarray); free(sendbuffer); free(recvbuffer);
	free(section_length);

	MPI_Finalize();
}
