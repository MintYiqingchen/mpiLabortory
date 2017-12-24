#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <limits.h>

int* local_str2int(char* argv[], int lo, int localsize){
	int* array = (int*)malloc(localsize*sizeof(int));
	int i;
	for(i = 0; i < localsize; i++){
		array[i] = atoi(argv[lo+i]);
	}
	return array;
}

int partition(int* array, int len){
	int pivot = array[0], lo = 0, hi = len-1;
	while(lo < hi){
		while(lo < hi && array[hi] > pivot) hi--;
		while(lo < hi && array[lo] <= pivot) lo++;
		if (lo!=hi)
		{
			array[lo] = array[lo]^array[hi];
			array[hi] = array[lo]^array[hi];
			array[lo] = array[lo]^array[hi];
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
void quicksort(int *array, int lo, int hi){
	int len = hi - lo;
	if(len <= 1) return;
	
	int *newp = array + lo;
	int pidx = partition(newp, len);
	quicksort(newp, 0, pidx);
	quicksort(newp, pidx+1, len);
}

void mul_mergesort(int* array, int* displs, int segnum, int len){
	/*
	 @displs: displs[i] is beginning of i-th section
	 @segnum: the number of segments
	 @len: length of array
	*/
	int i;
	int* ary2 = malloc(sizeof(int) * segnum);
	memcpy(ary2, array, len*sizeof(int));
	// for(i=0;i<len;i++)
	// 	printf("%d\n", ary2[i]); 
	
	int** begins = malloc(sizeof(int*)*segnum), **ends = malloc(sizeof(int*)*segnum);
	for(i = 0; i < segnum; i++){
		begins[i] = ary2 + displs[i];
		if(i==segnum-1)
			ends[i] = begins[i] + len - displs[segnum-1];
		else
			ends[i] = begins[i] + displs[i+1] - displs[i];
	}
	for(i=0;i<segnum;i++)
		printf("begin %ld end %ld\n", begins[i], ends[i]); 

	int glbidx = 0, finish = 0;
	while(!finish){
		int minelem = INT_MAX, minpointidx;
		for (i = 0; i < segnum; ++i)
		{
			if(begins[i] != ends[i] && minelem > *begins[i]){
				minelem = *begins[i];
				minpointidx = i;
			}
		}
		array[glbidx] = minelem; glbidx++;
		begins[minpointidx]++;

		finish = 1;
		for (i = 0; i < segnum; ++i)
		{
			if(begins[i] != ends[i]){
				finish = 0; break;
			}
		}
	}
	free(ary2);
	free(begins);
	free(ends);
}
int main(int argc, char* argv[]){
	int sz = argc-1, i;
	int* res = local_str2int(argv, 1, sz);

	int idx = partition(res, sz);
	quicksort(res, 0, idx);
	quicksort(res, idx+1, sz);
	for(i=0;i<sz;i++)
		printf("%d\n", res[i]); 
	int displs[5] = {0, 2, 4 ,6, 6};
	mul_mergesort(res, displs, 5, 10);
	for(i=0;i<sz;i++)
		printf("%d\n", res[i]); 
}