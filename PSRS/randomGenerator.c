#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
    if (argc<=1){
        return 0;
    }
    long long n = atoll(argv[1]);
    printf("the scale of numbers:%lld\n", n);
    FILE* f = fopen("randomnum.txt","a");
    srand(time(NULL));
    int i=0;
    for(i = 0; i<n;i++){
        fprintf(f,"%d\n", rand());
    }
    fclose(f);
}
