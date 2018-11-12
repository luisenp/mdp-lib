#!/bin/bash
#Usage ./run_testsolver.sh [problem] [nsims] [reps] [verbosity] [min_time] [max_time] [other_flags] [algorithm]

problem=$1
nsims=$2
reps=$3
verbosity=$4
min_time=$5
max_time=$6
other_flags=$7
algorithm=$8

echo "../testsolver.out $problem --heuristic=hmin --precompute-h --n=$nsims --reps=$reps --v=$verbosity --algorithm=$algorithm --min_time=$min_time --max_time=$max_time $other_flags"

../testsolver.out $problem \
  --heuristic=hmin --precompute-h \
  --n=$nsims --reps=$reps --v=$verbosity \
  --algorithm=$algorithm \
  --min_time=$min_time --max_time=$max_time $other_flags