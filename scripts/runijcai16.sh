#!/bin/bash

tracks=(known/square-4-error known/square-5-error \
        known/ring-5-error known/ring-6-error)
nsims=100
verbosity=1

for track in ${tracks[@]}; do
#   echo "${track}|lrtdp"
#   ../testsolver.out --track=../data/tracks/$track.track \
#   --algorithm=lrtdp --v=0 --n=1 \
#   --heuristic=hmin --hmin-solve-all 
  
#   echo "${track}|flares"
#   ../testsolver.out --track=../data/tracks/$track.track \
#   --algorithm=flares --v=0 --n=1 --optimal  \
#   --heuristic=hmin --hmin-solve-all 
  
  # FLARES
  for horizon in `seq 0 1`; do
    echo "${track}|flares|hor=${horizon}"
      ../testsolver.out --track=../data/tracks/$track.track \
      --algorithm=flares --horizon=$horizon --v=$verbosity --n=$nsims \
       --heuristic=hmin --hmin-solve-all 
  done
       
  # HDP(i,j)
  for i in `seq 0 1`; do
    for j in `seq 0 1`; do
      echo "${track}|hdp(${i},${j})"
        ../testsolver.out --track=../data/tracks/$track.track \
        --algorithm=hdp --i=$i --j=$j --v=$verbosity --n=$nsims \
        --heuristic=hmin --hmin-solve-all 
    done
  done
  
  # SSiPP(t)
  t=2
  for i in `seq 0 2`; do
    echo "${track}|ssipp(t)|j=${t}"
      ../testsolver.out --track=../data/tracks/$track.track \
      --algorithm=ssipp --horizon=$t --v=$verbosity --n=$nsims \
      --heuristic=hmin --hmin-solve-all 
    let "t *= 2"
  done
  
done