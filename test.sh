#!/bin/bash
declare -a ary

echo "start generate random numbers"
for ((i=1; i<=100000; i++))
do
    ary[i]=$RANDOM
done

echo "${ary[*]}"|xargs -r ./a.out

mpicc -g parallel_final.c -o final.out
echo "start mpi program"
echo "${ary[*]}"|xargs -r mpirun ./final.out
