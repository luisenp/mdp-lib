#!/bin/bash
# Usage ./run_ijcai18_ppddl_slurm.sh

ppddl_folder=/home/lpineda/mdp-lib/data/ppddl/ippc2008
domain=ex-blocksworld-fixed

for i in {01..10}; do
  problem=p$i
  sbatch --output=/home/lpineda/results_ijcai18/ppddl/${domain}_${problem}.txt \
    run_testppddl_softflares.sh ${ppddl_folder} ${domain} ${problem}
done

