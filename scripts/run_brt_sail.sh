#!/bin/bash

sizes=(10 20)
nsims=100

for size in ${sizes[@]}; do
  # LRTDP
  echo "${size}|lrtdp"
  ../testsolver.out --sailing-size=20 --sailing-goal=10 --algorithm=lrtdp \
  --horizon=2 --i=0 --j=1 --n=1 --v=100 --heuristic=hmin --hmin-solve-all
  
  # Approximate MLRDP
  for horizon in `seq 0 2`; do
    echo "${size}|mlrtdp|hor=${horizon}"
    ../testsolver.out --sailing-size=20 --sailing-goal=10 --algorithm=mlrtdp \
    --horizon=$horizon --n=$nsims --v=0 --heuristic=hmin --hmin-solve-all
  done
    
  # Approximate HDP(0,j)
  for i in `seq 0 1`; do
    for j in `seq 0 1`; do
      echo "${size}|hdp|hor=${horizon}"
      ../testsolver.out --sailing-size=20 --sailing-goal=10 --algorithm=hdp \
        --horizon=$horizon --n=$nsims --v=0 -i=$i j=$j \
        --heuristic=hmin --hmin-solve-all
    done
  done   
  
  # Approximate SSiPP
  horizon=1
  for i in `seq 0 3`; do
    echo "${size}|ssipp|hor=${horizon}"
    ../testsolver.out --sailing-size=20 --sailing-goal=10 --algorithm=hdp \
      --horizon=$horizon --n=$nsims --v=0 -i=$i j=$j \
      --heuristic=hmin --hmin-solve-all      
      let "horizon *= 2"
  done
  
done