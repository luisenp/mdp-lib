#!/bin/bash

for i in `seq 1 100`;
do
    ../testconc lao race ../data/tracks/roads-monster/map01.track 0 50 &>> ../experiments/race-all-lao.txt
done

for i in `seq 1 50`;
do
    ../testconc lao race ../data/tracks/roads-monster/map01.track 0 100 &>> ../experiments/race-all-lao.txt
done

for i in `seq 1 50`;
do
    ../testconc lao race ../data/tracks/roads-monster/map01.track 0 200 &>> ../experiments/race-all-lao.txt
done

for i in `seq 1 50`;
do
    ../testconc lao race ../data/tracks/roads-monster/map01.track 0 400 &>> ../experiments/race-all-lao.txt
done

for i in `seq 1 50`;
do
    ../testconc lao race ../data/tracks/roads-monster/map01.track 0 800 &>> ../experiments/race-all-lao.txt
done
