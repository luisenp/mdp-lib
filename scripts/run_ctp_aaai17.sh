#!/bin/bash

nsims=100
verbosity=1

# FLARES(0)
# for i in `seq 1 9`; do
#   echo "flares(0), test0${i}_10"
#   ../testsolver.out --problem=ctp \
#   --ctp=../data/ctps/small-graphs/test0${i}_10.graph \
#   --algorithm=flares --v=1 --dont-generate --horizon=0 --n=100
# done

# SSiPP(1)
# for i in `seq 1 9`; do
#   echo "ssipp(1), test0${i}_10"
#   ../testsolver.out --problem=ctp \
#   --ctp=../data/ctps/small-graphs/test0${i}_10.graph \
#   --algorithm=ssipp --v=1 --dont-generate --horizon=1 --n=100
# done

# HDP(0,0)
# for i in `seq 1 9`; do
#   echo "hdp(1), test0${i}_10"
#   ../testsolver.out --problem=ctp \
#   --ctp=../data/ctps/small-graphs/test0${i}_10.graph \
#   --algorithm=hdp --v=1 --dont-generate --i=0 --n=100
# done

# FLARES(1)
# for i in `seq 1 9`; do
#   echo "flares(1), test0${i}_10"
#   ../testsolver.out --problem=ctp \
#   --ctp=../data/ctps/small-graphs/test0${i}_10.graph \
#   --algorithm=flares --v=1 --dont-generate --horizon=1 --n=100
# done

# SSiPP(2)
for i in `seq 1 9`; do
  echo "ssipp(1), test0${i}_10"
  ../testsolver.out --problem=ctp \
  --ctp=../data/ctps/small-graphs/test0${i}_10.graph \
  --algorithm=ssipp --v=1 --dont-generate --horizon=2 --n=100
done
