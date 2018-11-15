#!/bin/bash
# Usage ./run_soft_flares_step_slurm.sh 

nsims=1000
reps=1
verbosity=-1
min_time=-1
max_time=-1
other_flags="--reset-every-trial"

problems=( "--track=../data/tracks/known/square-4-error.track --perror=0.10 --pslip=0.20" \
           "--track=../data/tracks/known/ring-5-error.track --perror=0.10 --pslip=0.20" \
           "--sailing-size=40 --sailing-goal=39" \
           "--sailing-size=40 --sailing-goal=20")

problems_str=(square4 ring5 sailing-corner sailing-middle)
alphas=(0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9)

output_dir="/home/lpineda/results_aamas19/until_stops_${nsims}_sims/flares_soft"
swarm_flags="--partition=longq --time=10-01:00:00"
for ((ip = 0; ip < ${#problems[@]}; ip++)); do
  problem=${problems[$ip]}
  problem_str=${problems_str[$ip]}  
  for alpha in ${alphas[@]}; do
    for horizon in `seq 1 4`; do
      sbatch ${swarm_flags} --output=${output_dir}/${problem_str}_"${alpha}_${horizon}".txt \
        run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time "$other_flags" \
        "soft-flares --labelf=step --dist=depth --horizon=$horizon --alpha=${alpha}"
    done
  done    
done
