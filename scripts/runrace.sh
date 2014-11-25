#!/bin/bash

for i in `seq 1 250`;
do
    ../testconc lao race ../data/tracks/roads-monster/map01.track - 250 &>> ../experiments/race-exp2-lao.txt
done

for i in `seq 1 250`;
do
    ../testconc wlao race ../data/tracks/roads-monster/map01.track - 250 &>> ../experiments/race-exp2-wlao50.txt
done

for i in `seq 1 250`;
do
    ../testconc lrtdp race ../data/tracks/roads-monster/map01.track - 250 &>> ../experiments/race-exp2-lrtdp.txt
done

for i in `seq 1 250`;
do
    ../testconc det race ../data/tracks/roads-monster/map01.track - 250 &>> ../experiments/race-exp2-det.txt
done