#!/bin/bash

sizes=(20 40)
nsims=100

for size in ${sizes[@]}; do
  let "goal = size/2"
  # LRTDP
#   echo "${size}|$goal|lrtdp"
#   ../testsolver.out --sailing-size=$size --sailing-goal=$goal --algorithm=lrtdp \
#   --horizon=2 --i=0 --j=1 --n=1 --v=100 --heuristic=hmin --hmin-solve-all
  
  # Approximate MLRDP
#   for horizon in `seq 0 2`; do
#     echo "${size}|$goal||mlrtdp|hor=${horizon}"
#     ../testsolver.out --sailing-size=$size --sailing-goal=$goal --algorithm=mlrtdp \
#     --horizon=$horizon --n=$nsims --v=0 --heuristic=hmin --hmin-solve-all
#   done
    
  # Approximate HDP(0,j)
#   for i in `seq 0 1`; do
#     for j in `seq 0 1`; do
#       echo "${size}|$goal||hdp|i=${i},j={$j}"
#       ../testsolver.out --sailing-size=$size --sailing-goal=$goal --algorithm=hdp \
#         --horizon=$horizon --n=$nsims --v=0 -i=$i j=$j \
#         --heuristic=hmin --hmin-solve-all
#     done
#   done   
  
  # Approximate SSiPP
  
#   horizon=1
#   for i in `seq 0 3`; do
#     echo "${size}|$goal||ssipp|hor=${horizon}"
#     ../testsolver.out --sailing-size=$size --sailing-goal=$goal --algorithm=ssipp \
#       --horizon=$horizon --n=$nsims --v=0 -i=$i j=$j \
#       --heuristic=hmin --hmin-solve-all
#       let "horizon *= 2"
#   done
  
  let "goal = size - 1"
  
  horizon=4
  for i in `seq 0 1`; do
    echo "${size}|$goal||ssipp|hor=${horizon}"
    ../testsolver.out --sailing-size=$size --sailing-goal=$goal --algorithm=ssipp \
      --horizon=$horizon --n=$nsims --v=0 -i=$i j=$j \
      --heuristic=hmin --hmin-solve-all
      let "horizon *= 2"
  done
done