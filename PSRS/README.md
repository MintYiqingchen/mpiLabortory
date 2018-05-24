# 正则采样排序测试指南
1. 生成测试数据，例如生成200000000个随机数
···
gcc randomGenerator.c -o randomG.out
for((i=0; i<20; i++))
do
./randomG.out 10000000
done
···

2. 运行正常的快速排序
···
gcc $(pwd)/parallel_ancillary.c
./a.out 200000000 randomnum.txt
···
3. 运行正则采样排序
···
mpicc -g parallel_final.c -o final.out
echo "start mpi program"
mpirun -np 8 final.out 200000000 randomnum.txt
···

## 测试结果
测试结果都是前面是快速排序，后面是正则采样排序

1. 100000个数字
![1000000排序][../images/100000_sort.png “sort time”]

2. 200000000个数字
![200000000排序][../images/200000000_sort.png "sort time"]