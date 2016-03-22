#!/bin/bash

tracks=(known/barto-big-error known/square-4-error known/square-5-error \
        known/ring-5-error known/ring-6-error)
nsims=100

for track in ${tracks[@]}; do
  echo "${track}|lrtdp"
  ../testsolver.out --track=../data/tracks/$track.track \
  --algorithm=lrtdp --v=0 --n=1 --no-initial-plan \
  --heuristic=hmin --hmin-solve-all 
  
  echo "${track}|mlrtdp"
  ../testsolver.out --track=../data/tracks/$track.track \
  --algorithm=mlrtdp --v=0 --n=1 --optimal --no-initial-plan \
  --heuristic=hmin --hmin-solve-all 
  
  # Approximate MLRDP 
  echo "${track}|mlrtdp|hor=0"
  ../testsolver.out --track=../data/tracks/$track.track \
  --algorithm=mlrtdp --horizon=0 --v=0 --n=$nsims --no-initial-plan \
  --heuristic=hmin --hmin-solve-all 
      
  horizon=1
  for i in `seq 0 3`; do
    echo "${track}|mlrtdp|hor=${horizon}"
      ../testsolver.out --track=../data/tracks/$track.track \
      --algorithm=mlrtdp --horizon=$horizon --v=0 --n=$nsims \
      --no-initial-plan --heuristic=hmin --hmin-solve-all 
    let "horizon *= 2"
  done
  
  # Approximate HDP(0,j) 
  echo "${track}|hdp(0,j)|j=0"
  ../testsolver.out --track=../data/tracks/$track.track \
  --algorithm=mlrtdp --horizon=0 --v=0 --n=$nsims --no-initial-plan \
  --heuristic=hmin --hmin-solve-all 
      
  j=1
  for i in `seq 0 3`; do
    echo "${track}|hdp(0,j)|j=${j}"
      ../testsolver.out --track=../data/tracks/$track.track \
      --algorithm=hdp --i=0 --j=$j --v=0 --n=$nsims \
      --no-initial-plan --heuristic=hmin --hmin-solve-all 
    let "j *= 2"
  done
  
done