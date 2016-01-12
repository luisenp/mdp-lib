#!/bin/bash

GRID=../data/gws/map6.gw
NSIMS=100000
TMAX=10

# for i in `seq 1 10`;
# do
#   ../simulmeta --grid=$GRID --meta=assume1 --nsims=$NSIMS --v=0 --steps_action=$i --steps_nop=$i --all_actions
# done
# 
# for i in `seq 1 10`;
# do
#   ../simulmeta --grid=$GRID --meta=assume2 --nsims=$NSIMS --v=0 --steps_action=$i --steps_nop=$i
# done
# for i in `seq 1 10`;
# do
#   ../simulmeta --grid=$GRID --meta=change --nsims=$NSIMS --v=0 --steps_action=$i --steps_nop=$i
# done
# 
for i in `seq 1 10`;
do
  ../simulmeta --grid=$GRID --meta=assume1 --nsims=$NSIMS --v=0 --steps_action=$i --steps_nop=$i
done

for i in `seq 1 10`;
do
  ../simulmeta --grid=$GRID --meta=nometa --nsims=$NSIMS --v=0 --steps_action=$i --steps_nop=1
done

for i in `seq 1 10`;
do
  ../simulmeta --grid=$GRID --meta=optimal --nsims=$NSIMS --v=0 --steps_action=$i --steps_nop=$i
done

for i in `seq 1 10`;
do
  ../simulmeta --grid=$GRID --meta=qvimprov --nsims=$NSIMS --v=0 --steps_action=$i --steps_nop=$i
done