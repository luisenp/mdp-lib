#!/bin/bash

# for i in `seq 1 20`;
# do
#     ../testconc det ctp ../data/ctps/AAAI-graphs/test00_20_T.graph - 250 &>> ../experiments/ctp-exp2-det.txt
# done

# for i in `seq 1 20`;
# do
#     ../testconc lao ctp ../data/ctps/AAAI-graphs/test00_20_T.graph 0 500 &>> ../experiments/ctp-exp2-lao.txt
#     sleep 10s
# done

for i in `seq 1 20`;
do
    ../testconc wlao ctp ../data/ctps/AAAI-graphs/test00_20_T.graph 0 500 &>> ../experiments/ctp-exp2-wlao20.txt
    sleep 10s
done
