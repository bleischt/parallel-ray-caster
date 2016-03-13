#!/bin/sh

exe=./cast

echo making...
make &>/dev/null


echo testing small bunny...
$exe bunny_small.in &>/dev/null
if ! diff -q image.ppm bunny_small.ppm
then 
   echo TEST FAILED
fi


echo testing large bunny...
$exe bunny_large.in &>/dev/null
if ! diff -q image.ppm bunny_large.ppm
then 
   echo TEST FAILED
fi


echo testing dragon...
$exe dragon.in &>/dev/null
if ! diff -q image.ppm dragon.ppm
then 
   echo TEST FAILED
fi



