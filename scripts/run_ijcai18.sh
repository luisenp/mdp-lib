reps=10
nsims=100


###################################################################
#                        RACETRACK DOMAIN                         #
###################################################################
#############
#  SQUARE 4 #
#############
track=known/square-4-error.track
min_time=1000
max_time=50000
# HPD(0,0)
echo "${track} | hdp(0,0)"
../testsolver.out --track=../data/tracks/$track \
--perror=0.25 --pslip=0.50 --heuristic=hmin --hmin-solve-all \
--n=$nsims --reps=$reps --v=-1 \
--algorithm=hdp --i=0 --j=0 \
--min_time=$min_time --max_time=$max_time

# FLARES(1)  
echo "${track} | flares(1)"
../testsolver.out --track=../data/tracks/$track \
--perror=0.25 --pslip=0.50 --heuristic=hmin --hmin-solve-all \
--n=$nsims --reps=$reps --v=-1 \
--algorithm=soft-flares --labelf=step --dist=depth --horizon=1 --alpha=0 \
--min_time=$min_time --max_time=$max_time

# Soft-FLARES
labelfuns=(linear exp logistic)
distfuns=(depth traj)
for labelf in ${labelfuns[@]}; do
  for distf in ${distfuns[@]}; do
    for horizon in `seq 0 3`; do
      echo "${track} | soft-flares-$distf-$labelf($horizon)"
      ../testsolver.out --track=../data/tracks/$track \
      --perror=0.25 --pslip=0.50 --heuristic=hmin --hmin-solve-all \
      --n=$nsims --reps=$reps --v=-1 \
      --algorithm=soft-flares --labelf=$labelf --dist=$distf \
      --horizon=$horizon --alpha=0.1 \
      --min_time=$min_time --max_time=$max_time
    done
  done
done

#############
#   RING 5  #
#############
track=known/ring-5-error.track
min_time=500
max_time=20000
# HPD(0,0)
echo "${track} | hdp(0,0)"
../testsolver.out --track=../data/tracks/$track \
--perror=0.25 --pslip=0.50 --heuristic=hmin --hmin-solve-all \
--n=$nsims --reps=$reps --v=-1 \
--algorithm=hdp --i=0 --j=0 \
--min_time=$min_time --max_time=$max_time

# FLARES(1)  
echo "${track} | flares(1)"
../testsolver.out --track=../data/tracks/$track \
--perror=0.25 --pslip=0.50 --heuristic=hmin --hmin-solve-all \
--n=$nsims --reps=$reps --v=-1 \
--algorithm=soft-flares --labelf=step --dist=depth --horizon=1 --alpha=0 \
--min_time=$min_time --max_time=$max_time

# Soft-FLARES
labelfuns=(linear exp logistic)
distfuns=(depth traj)
for labelf in ${labelfuns[@]}; do
  for distf in ${distfuns[@]}; do
    for horizon in `seq 0 3`; do
      echo "${track} | soft-flares-$distf-$labelf($horizon)"
      ../testsolver.out --track=../data/tracks/$track \
      --perror=0.25 --pslip=0.50 --heuristic=hmin --hmin-solve-all \
      --n=$nsims --reps=$reps --v=-1 \
      --algorithm=soft-flares --labelf=$labelf --dist=$distf \
      --horizon=$horizon --alpha=0.1 \
      --min_time=$min_time --max_time=$max_time
    done
  done
done

###################################################################
#                        SAILING  DOMAIN                          #
###################################################################
##############
#   MIDDLE   #
##############
min_time=200
max_time=5000
# HPD(0,0)
echo "sailing-30-middle | hdp(0,0)"
../testsolver.out --sailing-size=30 --sailing-goal=15
--perror=0.25 --pslip=0.50 --heuristic=hmin --hmin-solve-all \
--n=$nsims --reps=$reps --v=-1 \
--algorithm=hdp --i=0 --j=0 \
--min_time=$min_time --max_time=$max_time

# FLARES(1)  
echo "sailing-30-middle | flares(1)"
../testsolver.out --sailing-size=30 --sailing-goal=15
--perror=0.25 --pslip=0.50 --heuristic=hmin --hmin-solve-all \
--n=$nsims --reps=$reps --v=-1 \
--algorithm=soft-flares --labelf=step --dist=depth --horizon=1 --alpha=0 \
--min_time=$min_time --max_time=$max_time

# Soft-FLARES
labelfuns=(linear exp logistic)
distfuns=(depth traj)
for labelf in ${labelfuns[@]}; do
  for distf in ${distfuns[@]}; do
    for horizon in `seq 0 3`; do
      echo "sailing-30-middle | soft-flares-$distf-$labelf($horizon)"
../testsolver.out --sailing-size=30 --sailing-goal=15
      --perror=0.25 --pslip=0.50 --heuristic=hmin --hmin-solve-all \
      --n=$nsims --reps=$reps --v=-1 \
      --algorithm=soft-flares --labelf=$labelf --dist=$distf \
      --horizon=$horizon --alpha=0.1 \
      --min_time=$min_time --max_time=$max_time
    done
  done
done


##############
#   CORNER   #
##############
min_time=500
max_time=10000
# HPD(0,0)
echo "sailing-30-corner | hdp(0,0)"
../testsolver.out --sailing-size=30 --sailing-goal=29
--perror=0.25 --pslip=0.50 --heuristic=hmin --hmin-solve-all \
--n=$nsims --reps=$reps --v=-1 \
--algorithm=hdp --i=0 --j=0 \
--min_time=$min_time --max_time=$max_time

# FLARES(1)  
echo "sailing-30-corner | flares(1)"
../testsolver.out --sailing-size=30 --sailing-goal=29
--perror=0.25 --pslip=0.50 --heuristic=hmin --hmin-solve-all \
--n=$nsims --reps=$reps --v=-1 \
--algorithm=soft-flares --labelf=step --dist=depth --horizon=1 --alpha=0 \
--min_time=$min_time --max_time=$max_time

# Soft-FLARES
labelfuns=(linear exp logistic)
distfuns=(depth traj)
for labelf in ${labelfuns[@]}; do
  for distf in ${distfuns[@]}; do
    for horizon in `seq 0 3`; do
      echo "sailing-30-corner | soft-flares-$distf-$labelf($horizon)"
../testsolver.out --sailing-size=30 --sailing-goal=29
      --perror=0.25 --pslip=0.50 --heuristic=hmin --hmin-solve-all \
      --n=$nsims --reps=$reps --v=-1 \
      --algorithm=soft-flares --labelf=$labelf --dist=$distf \
      --horizon=$horizon --alpha=0.1 \
      --min_time=$min_time --max_time=$max_time
    done
  done
done
