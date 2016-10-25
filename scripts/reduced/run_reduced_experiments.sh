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
# pddl_folder=/home/lpineda/Desktop/skeren
pddl_folder=../../data/ppddl/ippc2008
# pddl_folder=../../data/ppddl/ippc2006

# The domain name.
domain=triangle-tireworld
# domain=zenotravel
# domain=boxworld
# domain=blocksworld
# domain=ex-blocksworld
# domain=elevators

# The name of the problem used to learn the best determinization. 
problem=p01

# The exception bound to use. 
k=0

# The name of the domain file to determinize
domain_file_name=domain

# Creating all possible determinizations
./create_all_determinizations.py -d $pddl_folder/$domain/$domain_file_name.pddl \
  -o /tmp/$domain
  
num_det=`ls /tmp/${domain}_det*.pddl -l | wc -l`
let "num_det=$num_det-1"

# Evaluating all possible determinizations in the first problem of this domain
all_successes=""
all_costs=""
for i in `seq 0 $num_det`; do
  cat $pddl_folder/$domain/$domain_file_name.pddl \
    $pddl_folder/$domain/problems/p01.pddl > $pddl_folder/$domain/p01.pddl
  successes_and_costs=`./run_experiment.sh $pddl_folder $domain p01 $i $k \
    | tail -n 1`
  echo $successes_and_costs
  all_successes=$all_successes`echo $successes_and_costs | awk '{print $1}'`,
  all_costs=$all_costs`echo $successes_and_costs | awk '{print $2}'`,
  rm $pddl_folder/$domain/p01.pddl
done
echo $all_successes
echo $all_costs

# Getting the best determinizations
n=3
best_n_determinizations=`./get_index_n_best_results.py \\
  -s ${all_successes::-1} -c ${all_costs::-1} -n $n `
best_determinization=`echo $best_n_determinizations | { read x _ ; echo $x; }`

echo 'best determinization: '$best_determinization
# Solving all problems with the best determinization
for i in {01..10}; do
  cat $pddl_folder/$domain/$domain_file_name.pddl \
    $pddl_folder/$domain/problems/p$i.pddl > $pddl_folder/$domain/p$i.pddl
  echo `./run_experiment.sh $pddl_folder $domain p$i $best_determinization $k \\
    | tail -n 1`
  rm $pddl_folder/$domain/p$i.pddl
done
