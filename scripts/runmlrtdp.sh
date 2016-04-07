#!/bin/bash

tracks=(known/barto-big-error known/square-4-error known/square-5-error \
        known/ring-5-error known/ring-6-error)
nsims=100

for track in ${tracks[@]}; do
  echo "${track}|lrtdp"
  ../testsolver.out --track=../data/tracks/$track.track \
  --algorithm=lrtdp --v=0 --n=1 \
  --heuristic=hmin --hmin-solve-all 
  
  echo "${track}|mlrtdp"
  ../testsolver.out --track=../data/tracks/$track.track \
  --algorithm=mlrtdp --v=0 --n=1 --optimal  \
  --heuristic=hmin --hmin-solve-all 
  
  # Approximate MLRDP 
  for horizon in `seq 0 1`; do
    echo "${track}|mlrtdp|hor=${horizon}"
      ../testsolver.out --track=../data/tracks/$track.track \
      --algorithm=mlrtdp --horizon=$horizon --v=0 --n=$nsims \
       --heuristic=hmin --hmin-solve-all 
  done
    
  # Approximate HDP(0,j)
  echo "${track}|hdp(0,j)|j=0"
  ../testsolver.out --track=../data/tracks/$track.track \
  --algorithm=hdp --i=0 --j=0 --v=0 --n=$nsims  \
  --heuristic=hmin --hmin-solve-all 
   
  j=1
  for i in `seq 0 1`; do
    echo "${track}|hdp(0,j)|j=${j}"
      ../testsolver.out --track=../data/tracks/$track.track \
      --algorithm=hdp --i=0 --j=$j --v=0 --n=$nsims \
       --heuristic=hmin --hmin-solve-all 
    let "j *= 2"
  done
  
done