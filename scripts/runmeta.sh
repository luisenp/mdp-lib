#!/bin/bash

GRID=../data/gws/map3.gw

# for i in `seq 1 20`;
# do
#   ../simulmeta --grid=$GRID --meta=assume1 --nsims=10000 --v=0 --steps_action=$i --steps_nop=$i
# done
# 
# for i in `seq 1 20`;
# do
#   ../simulmeta --grid=$GRID --meta=assume2 --nsims=10000 --v=0 --steps_action=$i --steps_nop=$i
# done
# 
# for i in `seq 1 20`;
# do
#   ../simulmeta --grid=$GRID --meta=assume1 --nsims=10000 --v=0 --steps_action=$i --steps_nop=$i --all_actions
# done

for i in `seq 1 20`;
do
  ../simulmeta --grid=$GRID --meta=nometa --nsims=100000 --v=0 --steps_action=$i --steps_nop=1
done

# for i in `seq 1 20`;
# do
#   ../simulmeta --grid=$GRID --meta=change --nsims=10000 --v=0 --steps_action=$i --steps_nop=$i
# done

for i in `seq 1 20`;
do
  ../simulmeta --grid=$GRID --meta=optimal --nsims=100000 --v=0 --steps_action=$i --steps_nop=$i
done

for i in `seq 1 20`;
do
  ../simulmeta --grid=$GRID --meta=qvimprov --nsims=100000 --v=0 --steps_action=$i --steps_nop=$i
done