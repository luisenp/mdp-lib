#!/bin/bash
# Usage ./run_aaai19_slurm.sh 
# Read all comments below before using

nsims=1000
alpha=0.1
reps=1
verbosity=-1
min_time=-1
max_time=-1
# other_flags="--reset-every-trial --per_replan --online"
other_flags="--reset-every-trial"

problems=( "--track=../data/tracks/known/square-4-error.track --perror=0.10 --pslip=0.20" \
           "--track=../data/tracks/known/ring-5-error.track --perror=0.10 --pslip=0.20" \
           "--sailing-size=40 --sailing-goal=39" \
           "--sailing-size=40 --sailing-goal=20")
brtdp_ubs=(30 60 300 200)

problems_str=(square4 ring5 sailing-corner sailing-middle)

output_dir="/home/lpineda/results_aamas19/until_stops_${nsims}_sims"
swarm_flags="--partition=longq --time=10-01:00:00"
rhos=(0.0625 0.03125)
distfuns=(depth traj plaus)          
labelfuns=(linear exp logistic)
for ((ip = 0; ip < ${#problems[@]}; ip++)); do
  problem=${problems[$ip]}
  problem_str=${problems_str[$ip]}
  brtdp_ub=${brtdp_ubs[$ip]}
  
  # ---- BRTDP
  sbatch ${swarm_flags} --output=${output_dir}/${problem_str}_"brtdp".txt \
    run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time "$other_flags" \
    "brtdp --ub=${brtdp_ub}"
      
  # ---- LRTDP
  sbatch ${swarm_flags} --output=${output_dir}/${problem_str}_"lrtdp".txt \
    run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time "$other_flags" \
    "lrtdp"
  
  # ---- RTDP (only if max time allowed, otherwise it will infinite loop)
  if [[ $max_time != "-1" ]]; then
    sbatch ${swarm_flags} --output=${output_dir}/${problem_str}_"rtdp".txt \
      run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time "$other_flags" \
      "rtdp"
  fi  
    
   # ---- HDP
  for hdp_i in `seq 0 3`; do
    # HDP(i)
    sbatch ${swarm_flags} --output=${output_dir}/${problem_str}_"hdp_${hdp_i}".txt \
      run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time "$other_flags" \
      "hdp --i=${hdp_i}"
    # HDP(i, 0)
    sbatch ${swarm_flags} --output=${output_dir}/${problem_str}_"hdp_${hdp_i}0".txt \
      run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time "$other_flags" \
      "hdp --j=0 --i=${hdp_i}" 
  done
    
  # ---- Trajectory-based SSiPP
  for rho in ${rhos[@]}; do
    sbatch ${swarm_flags} --output=${output_dir}/${problem_str}_"ssipp_$rho".txt \
      run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time "$other_flags" \
      "ssipp --rho=$rho"
      
    # Trajectory-based labeled-SSiPP
    sbatch ${swarm_flags} --output=${output_dir}/${problem_str}_"labeled_ssipp_$rho".txt \
      run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time "$other_flags" \
      "labeled-ssipp --rho=$rho"
  done
  
  # ---- SSiPP and FLARES
  ssipp_hor=1
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
    
  # ---- Soft-FLARES(0)
  sbatch ${swarm_flags} --output=${output_dir}/${problem_str}_"soft-flares_0".txt \
    ./run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time "$other_flags" \
    "soft-flares --labelf=linear --dist=depth --horizon=0 --alpha=$alpha"
  
  # ---- Soft-FLARES(1)
  # In this case the label function is irrelevant because
  # all of them result in the same set of labeling probabilities.
  # However, the distance function can potentially matter.   
  for distf in ${distfuns[@]}; do
    sbatch ${swarm_flags} --output=${output_dir}/${problem_str}_"soft-flares_1_$distf".txt \
      run_testsolver.sh "$problem" $nsims $reps $verbosity $min_time $max_time "$other_flags" \
      "soft-flares --labelf=linear --dist=$distf --horizon=1 --alpha=$alpha"
  done
  
  # ---- Soft-FLARES([2,4])
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

