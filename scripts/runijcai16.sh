#!/bin/bash

nsims=100
verbosity=1

tracks=(known/square-4-error known/square-5-error \
        known/ring-5-error known/ring-6-error)
        
# ######## Racetrack domain problems # ########
for track in ${tracks[@]}; do
  echo "${track}|lrtdp"
  ../testsolver.out --track=../data/tracks/$track.track \
  --algorithm=lrtdp --v=$verbosity --n=$nsims \
  --heuristic=hmin --precompute-h
  
  # FLARES
  for horizon in `seq 0 1`; do
    echo "${track}|flares(${horizon})"
      ../testsolver.out --track=../data/tracks/$track.track \
      --algorithm=flares --horizon=$horizon --v=$verbosity --n=$nsims \
       --heuristic=hmin --precompute-h 
  done
       
  # HDP(i,j)
  for i in `seq 0 1`; do
    for j in `seq 0 1`; do
      echo "${track}|hdp(${i},${j})"
        ../testsolver.out --track=../data/tracks/$track.track \
        --algorithm=hdp --i=$i --j=$j --v=$verbosity --n=$nsims \
        --heuristic=hmin --precompute-h 
    done
  done
  
  # SSiPP(t)
  t=2
  for i in `seq 0 2`; do
    echo "${track}|ssipp(${t})"
      ../testsolver.out --track=../data/tracks/$track.track \
      --algorithm=ssipp --horizon=$t --v=$verbosity --n=$nsims \
      --heuristic=hmin --precompute-h 
    let "t *= 2"
  done  
done

# ######## Sailing domain problems # ########
sizes=(20 40)
for size in ${sizes[@]}; do
  let "goal = size - 1"
  # LRTDP
  echo "${size}-$goal|lrtdp"
  ../testsolver.out --sailing-size=$size --sailing-goal=$goal --algorithm=lrtdp \
  --n=$nsims --v=$verbosity --heuristic=hmin --precompute-h --no-initial-plan
  
  # FLARES
  for horizon in `seq 0 1`; do
    echo "${size}-$goal|flares(${horizon})"
    ../testsolver.out --sailing-size=$size --sailing-goal=$goal --algorithm=flares \
    --horizon=$horizon --n=$nsims --v=$verbosity --heuristic=hmin --precompute-h
  done
    
  # HDP(i,j)
  for i in `seq 0 1`; do
    for j in `seq 0 1`; do
      echo "${size}-$goal|hdp(${i},${j})"
      ../testsolver.out --sailing-size=$size --sailing-goal=$goal --algorithm=hdp \
        --n=$nsims --v=$verbosity -i=$i j=$j \
        --heuristic=hmin --precompute-h
    done
  done   
  
  # Approximate SSiPP  
  horizon=4
  for i in `seq 0 1`; do
    echo "${size}-$goal||ssipp(${horizon})"
    ../testsolver.out --sailing-size=$size --sailing-goal=$goal --algorithm=ssipp \
      --horizon=$horizon --n=$nsims --v=$verbosity \
      --heuristic=hmin --precompute-h
      let "horizon *= 2"
  done
done
