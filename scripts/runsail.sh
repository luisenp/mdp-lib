#!/bin/bash

for i in `seq 1 250`;
do
    ../testconc det sail 50 20 250 &>> ../experiments/sail-exp2-det.txt
done

for i in `seq 1 250`;
do
    ../testconc lao sail 50 20 250 &>> ../experiments/sail-exp2-lao.txt
done

for i in `seq 1 250`;
do
    ../testconc wlao sail 50 20 250 &>> ../experiments/sail-exp2-wlao5.txt
done

for i in `seq 1 250`;
do
    ../testconc lrtdp sail 50 20 250 &>> ../experiments/sail-exp2-lrtdp.txt
done
