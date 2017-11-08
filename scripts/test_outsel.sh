# n=10000
# sz=8
# for ntr in `seq 10 10 100`; do
#   echo $ntr
#   ../testthts.out --randtree=$sz --backup=pb --action-sel=ucb1 --v=0 --n=$n --horizon=100 --out-sel=tran --trials=$ntr
#   ../testthts.out --randtree=$sz --backup=pb --action-sel=ucb1 --v=0 --n=$n --horizon=100 --out-sel=min-var --trials=$ntr
# done

heur=domain
tracks=(known/square-4-error known/ring-5-error)
nsims=400
horizon=25
actsel=greedy
for track in ${tracks[@]}; do
  echo $track
  for ntrials in `seq 10 10 200`; do
    echo $ntrials
    ../testthts.out --track=../data/tracks/$track.track --backup=pb --act-sel=$actsel --v=0 --n=$nsims --horizon=$horizon --out-sel=tran --trials=$ntrials --heur=$heur
    ../testthts.out --track=../data/tracks/$track.track --backup=pb --act-sel=$actsel --v=0 --n=$nsims --horizon=$horizon --out-sel=viub --trials=$ntrials --heur=$heur
    ../testthts.out --track=../data/tracks/$track.track --backup=pb --act-sel=$actsel --v=0 --n=$nsims --horizon=$horizon --out-sel=vpiu --trials=$ntrials --heur=$heur
  done
done