#!/bin/bash

for i in 100 500 1000 2000 5000 8000 10000 20000 50000 80000 100000
do
	echo $i
 	time SAMPLE_ITERATIONS=$i ./flip
done
