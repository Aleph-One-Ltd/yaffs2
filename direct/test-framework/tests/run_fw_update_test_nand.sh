#!/bin/bash

iterations=100000

[ -z $1 ]  || iterations=$1


rm -f iteration-max-*
touch iteration-max-$iterations

echo " Running $iterations iterations"
sleep 2

for ((i=0; i < $iterations; i++))  
do

   seed=$RANDOM   
   j=$(( $i % 10 ))

   rm -f log-nand-*$j
   rm -f seed-nand-*$j
   echo $seed>seed-nand-for-run-$i

   rm -f emfile-nand-*$j

   cp emfile-nand emfile-nand-$j

   echo "#########"
   echo "#########"
   echo "#########"
   echo "######### Run $i of $iterations with seed $seed"
   echo "#########"
   echo "#########"
   echo "#########"
   ./yaffs_test -u -f -p -s$seed -t 0 nand >log-nand-$i
done
