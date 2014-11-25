#!/bin/bash

for i in `seq 1 20`;
do
#     ../testconc lao sail 50 20 50
    ../testconc lao sail 50 20 50 &>> ../experiments/race-all-sail.txt
done

for i in `seq 1 20`;
do
    ../testconc lao sail 50 20 100 &>> ../experiments/race-all-sail.txt
done

for i in `seq 1 20`;
do
    ../testconc lao sail 50 20 200 &>> ../experiments/race-all-sail.txt
done

for i in `seq 1 20`;
do
    ../testconc lao sail 50 20 400 &>> ../experiments/race-all-sail.txt
done

for i in `seq 1 20`;
do
    ../testconc lao sail 50 20 800 &>> ../experiments/race-all-sail.txt
done
