#!/bin/bash


#set -x

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

   rm seed-m18-*$j
   echo $seed>seed-m18-for-run-$i


   rm -f emfile-m18-*$j
   cp emfile-m18 emfile-m18-$i

   rm -f log-m18-*$j

   echo "#########"
   echo "#########"
   echo "#########"
   echo "######### Run $i of $iterations with seed $seed"
   echo "#########"
   echo "#########"
   echo "#########"
   ./yaffs_test -u -f -p -s$seed -t0 M18-1 >log-m18-$i
done

