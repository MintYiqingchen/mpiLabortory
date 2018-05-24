#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <time.h>

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
	//int i;
	//for(i=0;i<len;i++)
	// 	printf("array %d\n", array[i]);
	//printf("lo %d\n", lo);
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

void mul_mergesort_slow(int* array, int* displs, int segnum, int len) {
	/*
	@displs: displs[i] is beginning of i-th section
	@segnum: the number of segments
	@len: length of array
	*/
	int i;
	int* ary2 = (int*)malloc(sizeof(int) * segnum);
	memcpy(ary2, array, len * sizeof(int));
	// for(i=0;i<len;i++)
	// 	printf("%d\n", ary2[i]);

	int** begins = (int**)malloc(sizeof(int*)*segnum), **ends = (int**)malloc(sizeof(int*)*segnum);
	for (i = 0; i < segnum; i++) {
		begins[i] = ary2 + displs[i];
		if (i == segnum - 1)
			ends[i] = begins[i] + len - displs[segnum - 1];
		else
			ends[i] = begins[i] + displs[i + 1] - displs[i];
	}
	for (i = 0; i<segnum; i++)
		printf("begin %ld end %ld\n", begins[i], ends[i]);

	int glbidx = 0, finish = 0;
	while (!finish) {
		int minelem = INT_MAX, minpointidx;
		for (i = 0; i < segnum; ++i)
		{
			if (begins[i] != ends[i] && minelem > *begins[i]) {
				minelem = *begins[i];
				minpointidx = i;
			}
		}
		array[glbidx] = minelem; glbidx++;
		begins[minpointidx]++;

		finish = 1;
		for (i = 0; i < segnum; ++i)
		{
			if (begins[i] != ends[i]) {
				finish = 0; break;
			}
		}
	}
	free(ary2);
	free(begins);
	free(ends);
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
	printf("lo:%d mid:%d hi:%d\n", lo, mid, hi);

	_binaryCut(array, begins, ends, lo, mid);
	_binaryCut(array, begins, ends, mid + 1, hi);
	merge(array, begins[lo], ends[mid], begins[mid + 1], ends[hi]);

	int* i;
	for (i = begins[lo]; i != ends[hi]; ++i)
	{
		printf("array: %d\n", *i);
	}
	printf("\n");
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
	for (i = 0; i<segnum; i++)
		printf("begin %ld end %ld\n", begins[i], ends[i]);

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
int main(int argc, char* argv[]) {
	int sz = argc - 1, i;
	clock_t start, end;
	if (sz<2)
	{
		printf("%d\n", sz);
		return 0;
	}
    long long num = atoll(argv[1]);
    /*FILE* fp = fopen(argv[2],"r");
//	int* res = local_str2int(argv, 1, sz);
    int* res = (int*)calloc(num, sizeof(int));
	for(i=0;i<num;i++){
        fscanf(fp,"%d", &res[i]);
    }
    fclose(fp);
*/

    int* res = (int*)calloc(num, sizeof(int));

    printf("generate random number at time:%f\n", clock()*1.0/CLOCKS_PER_SEC);
	for(i=0;i<num;i++){
        res[i]=rand();
    }
    printf("finish generate at %f\n", clock()*1.0/CLOCKS_PER_SEC);
	start = clock();
	int idx = partition(res, num);
	quicksort(res, 0, idx);
	quicksort(res, idx+1, num);
	end = clock();

    // int displs[5] = { 0, 2, 4 ,6, 6 };
	// mul_mergesort(res, displs, 5, sz);
	// mul_mergesort1(res, 3, 4);
	printf("------------------ after sort ----------------\n");
/*	for (i = 0; i < sz; i++)
		printf("%d\n", res[i]);
    */
	printf("The difference is: %f seconds\n",(end-start)*1.0/CLOCKS_PER_SEC);
	for (i = 0; i<num-1; i++){
		if(res[i]>res[i+1]){
			printf("didnt pass test on res[%d]:%d res[%d]:%d\n", i,i+1,res[i],res[i+1]);
		}
	}
	printf("test end\n");
	free(res);
	return 0;
}
