#!/bin/bash

nsims=100
verbosity=1

# ######## PROBLEMS WITH SIZE 10 # ######## 
# FLARES(0)
for i in `seq 0 9`; do
  echo "flares(0), test0${i}_10"
  ../testsolver.out --problem=ctp \
  --ctp=../data/ctps/small-graphs/test0${i}_10.graph \
  --algorithm=flares --v=$verbosity --dont-generate --horizon=0 --n=$nsims \
  --heuristic=hmin
done

# SSiPP(1)
for i in `seq 0 9`; do
  echo "ssipp(1), test0${i}_10"
  ../testsolver.out --problem=ctp \
  --ctp=../data/ctps/small-graphs/test0${i}_10.graph \
  --algorithm=ssipp --v=$verbosity --dont-generate --horizon=1 --n=$nsims \
  --heuristic=hmin
done

# HDP(0)
for i in `seq 0 9`; do
  echo "hdp(0), test0${i}_10"
  ../testsolver.out --problem=ctp \
  --ctp=../data/ctps/small-graphs/test0${i}_10.graph \
  --algorithm=hdp --v=$verbosity --dont-generate --i=0 --n=$nsims \
  --heuristic=hmin
done

# ######## PROBLEMS WITH SIZE 19 # ######## 
# # FLARES(0)
# for i in `seq 0 9`; do
#   echo "flares(0), test0${i}_19"
#   ../testsolver.out --problem=ctp \
#   --ctp=../data/ctps/small-graphs/test0${i}_19.graph \
#   --algorithm=flares --v=$verbosity --dont-generate --horizon=0 --n=$nsims
# done
# 
# # SSiPP(1)
# for i in `seq 0 9`; do
#   echo "ssipp(1), test0${i}_19"
#   ../testsolver.out --problem=ctp \
#   --ctp=../data/ctps/small-graphs/test0${i}_19.graph \
#   --algorithm=ssipp --v=$verbosity --dont-generate --horizon=1 --n=$nsims
# done
# 
# # HDP(0)
# for i in `seq 0 9`; do
#   echo "hdp(0), test0${i}_19"
#   ../testsolver.out --problem=ctp \
#   --ctp=../data/ctps/small-graphs/test0${i}_19.graph \
#   --algorithm=hdp --v=$verbosity --dont-generate --i=0 --n=$nsims
# done
