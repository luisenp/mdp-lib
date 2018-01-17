#!/bin/bash

#SBATCH --job-name=test_solver
#SBATCH --output=../results

problem=$1
nsims=$2
reps=$3
verbosity=$4
min_time=$5
max_time=$6
other_flags=$7
algorithm=$8

srun run_testsolver.sh $problem $nsims $reps $verbosity $min_time $max_time $other_flags $algorithm
