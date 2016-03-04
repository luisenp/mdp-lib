#!/bin/bash

# tracks=(square-4-error square-5-error ring-5-error ring-6-error)
tracks=(square-5-potholes)

for track in ${tracks[@]}; do
  echo "${track}|lrtdp"
  ../testrace.out --track=../data/tracks/known/$track.track \
  --algorithm=lrtdp --v=0 --n=100
  for horizon in `seq 0 4`; do
    for expansions in `seq 1 3`; do
        echo "${track}|${horizon}|${expansions}"
      ../testrace.out --track=../data/tracks/known/$track.track \
      --algorithm=epic --horizon=$horizon --expansions=$expansions \
      --v=0 --n=1000
    done    
  done
done