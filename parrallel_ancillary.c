#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

int* local_str2int(char* argv[], lo, localsize){
	int* array = (int*)malloc(localsize*sizeof(int));
	int i;
	for(i = 0; i < localsize; i++){
		array[i] = atoi(argv[lo+i]);
	}
	return array;
}

int main(){
	char* input[30];
	int sz = 10, i;
	for(i=0;i<sz;i++)
		scanf("%s", input[i]);
	int* res = local_str2int(input, 0, sz);
	for(i=0;i<sz;i++)
		printf("%d\n", res[i]); 
}