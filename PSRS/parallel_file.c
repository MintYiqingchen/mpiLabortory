#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "mpi.h"

int main(int argc, char* argv[]){
    MPI_Init(&argc, &argv);
    MPI_File fh;
    MPI_File_open(MPI_COMM_WORLD, "randomnum.txt",
            MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
    MPI_Status status;
    int buffer[15];
    int prank;
    MPI_Comm_rank(MPI_COMM_WORLD, &prank);
    MPI_File_read_at(fh, 10*sizeof(int)*prank, buffer, 10, MPI_INT, &status);
    int i;
    for(i=0; i<10;i++){
        printf("process %d [%d]:%d\n", prank, i, buffer[i]);
    }
    MPI_Finalize();
}
