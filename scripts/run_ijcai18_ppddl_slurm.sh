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
pddl_folder=$1

# The domain name.
domain=$2

# The problem name.
problem=$3

# Parameters
ntrials=1000
nsims=50
verbosity=0
depth=6
alpha=0.3
dist=traj
labelf=linear

# Starts the planning server to connect to mdpsim
time ../testppddl.out $pddl_folder/$domain/$problem.pddl $problem 1000 50 1000\
  --algorithm=soft-flares --depth=$depth --alpha=$alpha \
  --dist=$dist --labelf=$labelf &
