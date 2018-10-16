#!/bin/bash
#Usage ./run_testsolver.sh [track] [k_reduced] [nsims] [model] [maxt] [extraflags]

size=$1
let "goal = $1 - 1"
k_reduced=$2
nsims=$3
model=$4
maxt=$5
extraflags=$6
verbosity=$7

echo "--domain=sailing --sailing-size=${size} --sailing-goal=${goal} --k=${k_reduced} --n=${nsims} --${model} --maxt=${maxt} --v=${verbosity} --mds=-1 ${extraflags}"

../../testreduced.out --domain=sailing \
  --sailing-size=${size} --sailing-goal=${goal} \
  --k=${k_reduced} --n=${nsims} --${model} --maxt=${maxt} \
  --v=${verbosity} ${extraflags}
  