#!/bin/bash

# The folder where the PPDDL files are stored. The contents should be 
# structured as follows:
#
# pddl_folder/domain-name-1/*.pddl
# pddl_folder/domain-name-2/*.pddl
#
# The PPDDL files must contain both domain and problem definitions and
# the problem name in the file must match the PPDDL filename.
#
# pddl_folder=../../data/ppddl/ippc2008
pddl_folder=../../data/ppddl

# The domain name.
domain=triangle-tireworld-60

# The name of the problem used to learn the best determinization. 
problem=p01

# The exception bound to use.
klearn=0
k=0

# Creating all possible determinizations
./create_all_determinizations.py -d $pddl_folder/$domain/domain.pddl \
  -o /tmp/$domain
  
num_det=`ls /tmp/${domain}_det*.pddl -l | wc -l`
let "num_det=$num_det-1"

# Evaluating all possible determinizations in the first problem of this domain
all_successes=""
all_costs=""
for i in `seq 0 $num_det`; do
  successes_and_costs=`./run_experiment.sh $pddl_folder $domain p01 $i $klearn \
    | tail -n 1`
  echo $successes_and_costs
  all_successes=$all_successes`echo $successes_and_costs | awk '{print $1}'`,
  all_costs=$all_costs`echo $successes_and_costs | awk '{print $2}'`,
  sleep 10
done
echo $all_successes
echo $all_costs

# Getting the best determinizations
n=3
best_n_determinizations=`./get_index_n_best_results.py \\
  -s ${all_successes::-1} -c ${all_costs::-1} -n $n `
best_determinization=`echo $best_n_determinizations | { read x _ ; echo $x; }`
echo "best determinization :"$best_determinization

# Solving all problems with the best determinization
for i in {01..10}; do
  echo `time ./run_experiment.sh $pddl_folder $domain p$i $best_determinization $k \\
    | tail -n 1`
done
