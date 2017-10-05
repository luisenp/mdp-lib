n=10000
sz=8
for ntr in `seq 10 10 100`; do
  echo $ntr
  ../testthts.out --randtree=$sz --backup=pb --action-sel=ucb1 --v=0 --n=$n --horizon=100 --out-sel=tran --trials=$ntr
  ../testthts.out --randtree=$sz --backup=pb --action-sel=ucb1 --v=0 --n=$n --horizon=100 --out-sel=min-var --trials=$ntr
done