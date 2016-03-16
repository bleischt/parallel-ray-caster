#!/bin/sh

exe=./cast
#&>/dev/null

echo making...
make 


echo testing small bunny...
if [ "$1" = "time" ]
then
   time $exe bunny_small_binary.in 
else
   $exe bunny_small_binary.in 
fi
if ! diff -q image.ppm bunny_small.ppm
then 
   echo TEST FAILED
fi



echo testing large bunny...
if [ "$1" = "time" ]
then
   time $exe bunny_large_binary.in 
else
   $exe bunny_large_binary.in 
fi 
if ! diff -q image.ppm bunny_large.ppm
then 
   echo TEST FAILED
fi


echo testing dragon...
if [ "$1" = "time" ]
then
   time $exe dragon_binary.in 
else
   $exe dragon_binary.in 
fi 
if ! diff -q image.ppm dragon.ppm
then 
   echo TEST FAILED
fi



