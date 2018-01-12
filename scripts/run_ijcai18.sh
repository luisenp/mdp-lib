reps=10
nsims=100

tracks=(known/square-4-error known/ring-5-error)

for track in ${tracks[@]}; do
  #HPD(0,0)
  echo "${track}|hdp(0,0)"
  ../testsolver.out --track=data/tracks/known/$track.track \
  --perror=0.25 --pslip=0.50 --heuristic=hmin --hmin-solve-all \
  --algorithm=hdp,flares,soft-flares \
  --n=$nsims --reps=$reps --v=-1 --horizon=1 --alpha=0.1 --i=0 --j=0 \
  --dist=depth --labelf=exp

  #FLARES(1)  
    echo echo "${track}|hdp(0,0)"
  ../testsolver.out --track=data/tracks/known/$track.track \
  --perror=0.25 --pslip=0.50 --heuristic=hmin --hmin-solve-all \
  --algorithm=soft-flares --n=$nsims --reps=$reps --v=-1 --horizon=1 \
  --alpha=0 --dist=depth --labelf=step
  
  #Soft-FLARES depth-distance labelf=linear
  for horizon in `seq 0 3`; do
    echo echo "${track}|hdp(0,0)"
  done
  
  #SOFT-FLARES t=1 depth-distance labelf=linear
  ../testsolver.out --track=data/tracks/known/$track.track \
  --perror=0.25 --pslip=0.50 --heuristic=hmin --hmin-solve-all \
  --algorithm=soft-flares --n=$nsims --reps=$reps --v=-1 --horizon=1 \
  --alpha=0 --dist=depth --labelf=linear
  
  #SOFT-FLARES t=3 trajprob-distance labelf=linear
  ../testsolver.out --track=data/tracks/known/$track.track \
  --perror=0.25 --pslip=0.50 --heuristic=hmin --hmin-solve-all \
  --algorithm=soft-flares --n=$nsims --reps=$reps --v=-1 --horizon=1 \
  --alpha=0 --dist=traj --labelf=linear
done
