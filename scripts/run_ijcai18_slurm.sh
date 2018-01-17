#!/bin/bash
#Usage ./run_ijcai18_slurm.sh 

nsims=100
alpha=0.1
reps=1
verbosity=-1
min_time=10
max_time=10000
other_flags=""

# problems=( "--track=../data/tracks/known/square-2-error.track --perror=0.25 --pslip=0.50" \
#            "--track=../data/tracks/known/ring-5-error.track --perror=0.25 --pslip=0.50" \
#            "--sailing-size=40 --sailing-goal=39" \
#            "--sailing-size=40 --sailing-goal=20")
           
problems=( "--track=../data/tracks/known/square-3-error.track --perror=0.25 --pslip=0.50")
            
distfuns=(depth traj)          
labelfuns=(linear exp logistic)
for ((ip = 0; ip < ${#problems[@]}; ip++)); do
  problem=${problems[$ip]}
  
  # HDP(0,)
  ./run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time \
    "$other_flags" \
    "hdp --i=0 --j=0"
    
  # FLARES(1)
  ./run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time \
    "$other_flags" \
    "soft-flares --labelf=step --dist=depth --horizon=1 --alpha=$alpha"
    
  # Soft-FLARES(0)
  ./run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time \
    "$other_flags" \
    "soft-flares --labelf=linear --dist=depth --horizon=0 --alpha=$alpha"
  
  # Soft-FLARES(1)
  # In this case the label function is irrelevant because
  # all of them result in the same set of labeling probabilities.
  # However, the distance function can potentially matter.   
  for distf in ${distfuns[@]}; do
    ./run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time \
      "$other_flags" \
      "soft-flares --labelf=linear --dist=$distf --horizon=1 --alpha=$alpha"
  done
  
  # Soft-FLARES([2,4])
  labelfuns=(linear exp logistic)
  for labelf in ${labelfuns[@]}; do
    for distf in ${distfuns[@]}; do
      for horizon in `seq 2 4`; do
        ./run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time \
          "$other_flags" \
          "soft-flares --labelf=$labelf --dist=$distf --horizon=$horizon --alpha=$alpha"
      done
    done
  done
done

  
#     sbatch --output=results_ijcai18/racetrack/problem_$algorithm.txt run.slurm $domain $1 $2 $port
