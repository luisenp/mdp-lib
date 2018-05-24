#!/bin/bash
# Usage ./run_nips18_slurm.sh 
# Read all comments below before using

nsims=20000
alpha=0.1
reps=1
verbosity=0
min_time=-1
max_time=-1
other_flags="--reset-every-trial"

problems=( "--track=../data/tracks/known/square-4-error.track --perror=0.10 --pslip=0.20" \
           "--track=../data/tracks/known/ring-5-error.track --perror=0.10 --pslip=0.20" \
           "--sailing-size=40 --sailing-goal=39" \
           "--sailing-size=40 --sailing-goal=20")

problems_str=(square4 ring5 sailing-corner sailing-middle)

output_dir="/home/lpineda/results_nips18/20ksims"
swarm_flags="--partition=longq --time=10-01:00:00"
rhos=(0.0625 0.03125)
distfuns=(depth traj)          
labelfuns=(linear exp logistic)
for ((ip = 0; ip < ${#problems[@]}; ip++)); do
  problem=${problems[$ip]}
  problem_str=${problems_str[$ip]}
  
  # LRTDP
  sbatch ${swarm_flags} --output=${output_dir}/${problem_str}_"lrtdp".txt \
    run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time "$other_flags" \
    "lrtdp"
  
  # RTDP (only if max time allowed, otherwise it will infinite loop)
  if [[ $max_time != "-1" ]]; then
    sbatch ${swarm_flags} --output=${output_dir}/${problem_str}_"rtdp".txt \
      run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time "$other_flags" \
      "rtdp"
  fi  
  
  # HDP(0)
  sbatch ${swarm_flags} --output=${output_dir}/${problem_str}_"hdp_0".txt \
    run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time "$other_flags" \
    "hdp --i=0"
    
  # HDP(0,0)
  sbatch ${swarm_flags} --output=${output_dir}/${problem_str}_"hdp_00".txt \
    run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time "$other_flags" \
    "hdp --j=0 --i=0" 
  
  # HDP(1)
  sbatch ${swarm_flags} --output=${output_dir}/${problem_str}_"hdp_1".txt \
    run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time "$other_flags" \
    "hdp --i=1"
    
  # HDP(1,0)
  sbatch ${swarm_flags} --output=${output_dir}/${problem_str}_"hdp_10".txt \
    run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time "$other_flags" \
    "hdp --j=0 --i=1"
    
  # HDP(1,1)
  sbatch ${swarm_flags} --output=${output_dir}/${problem_str}_"hdp_11".txt \
    run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time "$other_flags" \
    "hdp --j=1 --i=1"
    
  # Trajectory-based SSiPP
  for rho in ${rhos[@]}; do
    sbatch ${swarm_flags} --output=${output_dir}/${problem_str}_"ssipp_$rho".txt \
      run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time "$other_flags" \
      "ssipp --rho=$rho"
      
    # Trajectory-based labeled-SSiPP
    sbatch ${swarm_flags} --output=${output_dir}/${problem_str}_"labeled_ssipp_$rho".txt \
      run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time "$other_flags" \
      "labeled-ssipp --rho=$horizon"
  done
  
  ssipp_hor=2
  for horizon in `seq 1 4`; do
    # Depth-based SSiPP(1-4)
    sbatch ${swarm_flags} --output=${output_dir}/${problem_str}_"ssipp_$ssipp_hor".txt \
      run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time "$other_flags" \
      "ssipp --horizon=$ssipp_hor"
    
    # Labeled-SSiPP
    sbatch ${swarm_flags} --output=${output_dir}/${problem_str}_"labeled_ssipp_$ssipp_hor".txt \
      run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time "$other_flags" \
      "labeled-ssipp --horizon=$ssipp_hor"
    let "ssipp_hor *= 2"
      
    # FLARES(1-4)
    sbatch ${swarm_flags} --output=${output_dir}/${problem_str}_"flares_$horizon".txt \
      run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time "$other_flags" \
      "soft-flares --labelf=step --dist=depth --horizon=$horizon --alpha=0"
  done
    
  # Soft-FLARES(0)
  sbatch ${swarm_flags} --output=${output_dir}/${problem_str}_"soft-flares_0".txt \
    ./run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time "$other_flags" \
    "soft-flares --labelf=linear --dist=depth --horizon=0 --alpha=$alpha"
  
  # Soft-FLARES(1)
  # In this case the label function is irrelevant because
  # all of them result in the same set of labeling probabilities.
  # However, the distance function can potentially matter.   
  for distf in ${distfuns[@]}; do
    sbatch ${swarm_flags} --output=${output_dir}/${problem_str}_"soft-flares_1_$distf".txt \
      run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time "$other_flags" \
      "soft-flares --labelf=linear --dist=$distf --horizon=1 --alpha=$alpha"
  done
  
  # Soft-FLARES([2,4])
  for labelf in ${labelfuns[@]}; do
    for distf in ${distfuns[@]}; do
      for horizon in `seq 2 4`; do
        sbatch ${swarm_flags} --output=${output_dir}/${problem_str}_"soft-flares_${horizon}_${distf}_${labelf}".txt \
          run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time "$other_flags" \
          "soft-flares --labelf=$labelf --dist=$distf --horizon=$horizon --alpha=$alpha"
      done
    done
  done
done

