#!/bin/bash
declare -a ary
rm randomnum.txt
gcc randomGenerator.c -o randomG.out
for((i=0; i<20; i++))
do
./randomG.out 10000000
done


gcc $(pwd)/parallel_ancillary.c
./a.out 200000000 randomnum.txt

mpicc -g parallel_final.c -o final.out
echo "start mpi program"
mpirun -np 8 final.out 200000000 randomnum.txt
