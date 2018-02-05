reps=10
nsims=1000
min_time=-1
max_time=-1
verbosity=-1
other_flags=""

problems=( "--track=../data/tracks/known/square-4-error.track --perror=0.25 --pslip=0.50" \
           "--track=../data/tracks/known/ring-5-error.track --perror=0.25 --pslip=0.50" \
           "--sailing-size=40 --sailing-goal=39" \
           "--sailing-size=40 --sailing-goal=20")
           
problems=(--track=../data/tracks/known/square-2-error.track)

for ((i = 0; i < ${#problems[@]}; i++)); do
  problem=${problems[$i]}
  # HPD(0,0)
  echo "${problem} | hdp(0,0)"
  ../testsolver.out $problem \
  --heuristic=hmin --hmin-solve-all \
  --n=$nsims --reps=$reps --v=$verbosity \
  --algorithm=hdp --i=0 --j=0 \
  --min_time=$min_time --max_time=$max_time $other_flags

  # FLARES(1)  
  echo "${problem} | flares(1)"
  ../testsolver.out $problem \
  --heuristic=hmin --hmin-solve-all \
  --n=$nsims --reps=$reps --v=$verbosity \
  --algorithm=soft-flares --labelf=step --dist=depth --horizon=1 --alpha=0 \
  --min_time=$min_time --max_time=$max_time $other_flags

  # Soft-FLARES(0)
  # In this the label and distance functions are irrelevant because
  # all of them result in the same set of labeling probabilities
  echo "${problem} | soft-flares(0)"
  ../testsolver.out $problem \
  --heuristic=hmin --hmin-solve-all \
  --n=$nsims --reps=$reps --v=$verbosity \
  --algorithm=soft-flares --labelf=linear --dist=depth \
  --horizon=0 --alpha=0.1 \
  --min_time=$min_time --max_time=$max_time $other_flags
  
  # Soft-FLARES(1)
  # In this the label function is irrelevant because
  # all of them result in the same set of labeling probabilities.
  # However, the distance function can potentially matter. 
  distfuns=(depth traj)
  for distf in ${distfuns[@]}; do
    echo "${problem} | soft-flares-$distf(1)"
    ../testsolver.out $problem \
    --heuristic=hmin --hmin-solve-all \
    --n=$nsims --reps=$reps --v=$verbosity \
    --algorithm=soft-flares --labelf=linear --dist=$distf \
    --horizon=1 --alpha=0.1 \
    --min_time=$min_time --max_time=$max_time $other_flags
  done

  # Soft-FLARES([2,4])
  labelfuns=(linear exp logistic)
  for labelf in ${labelfuns[@]}; do
    for distf in ${distfuns[@]}; do
      for horizon in `seq 2 4`; do
        echo "${problem} | soft-flares-$distf-$labelf($horizon)"
        ../testsolver.out $problem \
        --heuristic=hmin --hmin-solve-all \
        --n=$nsims --reps=$reps --v=$verbosity \
        --algorithm=soft-flares --labelf=$labelf --dist=$distf \
        --horizon=$horizon --alpha=0.1 \
        --min_time=$min_time --max_time=$max_time $other_flags
      done
    done
  done
done
