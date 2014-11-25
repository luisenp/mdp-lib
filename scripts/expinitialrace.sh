#!/bin/bash

for ta in {50..500..50}
do
  for i in `seq 1 100`;
  do
      ../testconc lao race ../data/tracks/roads-monster/map01.track - $ta 50 # &>> ../experiments/race-init-ta50.txt
  done
done