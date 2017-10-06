# n=10000
# sz=8
# for ntr in `seq 10 10 100`; do
#   echo $ntr
#   ../testthts.out --randtree=$sz --backup=pb --action-sel=ucb1 --v=0 --n=$n --horizon=100 --out-sel=tran --trials=$ntr
#   ../testthts.out --randtree=$sz --backup=pb --action-sel=ucb1 --v=0 --n=$n --horizon=100 --out-sel=min-var --trials=$ntr
# done

tracks=(known/square-2-error known/ring-3-error)
nsims=1000
for track in ${tracks[@]}; do
  echo $track
  for ntrials in `seq 10 10 200`; do
    echo $ntrials
    ../testthts.out --track=../data/tracks/$track.track --backup=pb --action-sel=ucb1 --v=0 --n=$nsims --horizon=50 --out-sel=tran --trials=$ntrials
    ../testthts.out --track=../data/tracks/$track.track --backup=pb --action-sel=ucb1 --v=0 --n=$nsimsn --horizon=50 --out-sel=min-var --trials=$ntrials
  done
done