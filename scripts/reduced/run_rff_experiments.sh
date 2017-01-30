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
pddl_folder=../../data/ppddl/ippc2008/

# The domain name.
domain=ex-blocksworld

# The name of the problem used to learn the best determinization. 
problem=p01

# Creating the most-likely outcome determinization
./create_mlo_determinization.py -d $pddl_folder/$domain/domain.pddl \
  -o /tmp/$domain

# Solving all problems with RFF
for i in {01..10}; do
  echo `time ./run_experiment.sh $pddl_folder $domain p$i 0 rff \\
    | tail -n 1`
done
