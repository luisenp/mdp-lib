#!/bin/bash

for i in `seq 1 20`;
do
  ../simulmeta --grid=../data/gws/map5.gw --meta=assume1 --nsims=10000 --v=0 --steps_action=$i --steps_nop=$i
done

for i in `seq 1 20`;
do
  ../simulmeta --grid=../data/gws/map5.gw --meta=assume2 --nsims=10000 --v=0 --steps_action=$i --steps_nop=$i
done

for i in `seq 1 20`;
do
  ../simulmeta --grid=../data/gws/map5.gw --meta=assume1 --nsims=10000 --v=0 --steps_action=$i --steps_nop=$i --all_actions
done

for i in `seq 1 20`;
do
  ../simulmeta --grid=../data/gws/map5.gw --meta=assume2 --nsims=10000 --v=0 --steps_action=$i --steps_nop=$i --all_actions
done

for i in `seq 1 20`;
do
  ../simulmeta --grid=../data/gws/map5.gw --meta=nometa --nsims=10000 --v=0 --steps_action=$i --steps_nop=1
done

for i in `seq 1 20`;
do
  ../simulmeta --grid=../data/gws/map5.gw --meta=change --nsims=10000 --v=0 --steps_action=$i --steps_nop=$i
done