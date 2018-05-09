#!/bin/bash
# Usage [pddl_folder] [domain] [problem]

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

# The problem number
problem=$3

# Parameters
ntrials=1000
nsims=50
verbosity=10
depth=10
alpha=0.1
dist=traj
labelf=logistic

timeout 20m time ../testppddl.out $pddl_folder/$domain/$problem.pddl $problem \
  $ntrials $nsims $verbosity --algorithm=soft-flares \
  --depth=$depth --alpha=$alpha --dist=$dist --labelf=$labelf --debug

