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

   rm -f log-nand_enc-*$j
   rm -f seed-nand_enc-*$j
   echo $seed>seed-nand_enc-for-run-$i

   rm -f emfile-nand_enc-*$j

   cp emfile-nand_enc emfile-nand_enc-$j

   echo "#########"
   echo "#########"
   echo "#########"
   echo "######### Run $i of $iterations with seed $seed"
   echo "#########"
   echo "#########"
   echo "#########"
   ./yaffs_test -u -f -p -s$seed -t 0 nand_enc >log-nand_enc-$i
done
