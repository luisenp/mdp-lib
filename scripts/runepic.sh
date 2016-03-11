#!/bin/bash

tracks=(known/barto-big-error known/square-4-error known/square-5-error \
        known/ring-5-error known/ring-6-error)
nsims=1

for track in ${tracks[@]}; do
  echo "${track}|lrtdp"
  for n in `seq 1 ${nsims}`; do
    ../testsolver.out --track=../data/tracks/$track.track \
    --algorithm=lrtdp --v=0 --n=100 --trials=10
  done
  for horizon in `seq 0 4`; do
    for expansions in `seq 1 3`; do
      echo "${track}|${horizon}|${expansions}"
      for n in `seq 1 ${nsims}`; do
        ../testsolver.out --track=../data/tracks/$track.track \
        --algorithm=epic --horizon=$horizon --expansions=$expansions \
        --v=0 --n=100 --trials=10
      done
    done    
  done
done
