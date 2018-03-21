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
depth=6
alpha=0.3
dist=traj
labelf=linear

# Starts the planning server to connect to mdpsim
../planserv $pddl_folder/$domain/$problem.pddl $problem \
  soft-flares --depth=$depth --alpha=$alpha --dist=$dist --labelf=$labelf &

# Starts the mdpsim server
../../mdpsim-2.2/mdpsim --port=2323 --time-limit=1200000 --round-limit=50 \
  --turn-limit=2500 $pddl_folder/$domain/$problem.pddl &

# This might not be necessary, but just in case
sleep 1

# Starts the mdpsim client
../../mdpsim-2.2/mdpclient --host=localhost --port=2323 \
  $pddl_folder/$domain/$problem.pddl &> log.txt

# Kill the planning and mdpsim servers
./kill_servers.sh

# Extract the number of successes and turns (cost) resulting from this 
# determinization
successes=`grep -o "<successes>[0-9.]*" log.txt | grep -o "[0-9.]*"`
cost=`grep -o "<turn-average>[0-9.]*" log.txt | grep -o "[0-9.]*"`
if [ -z "$cost" ]; then
  cost=10000.0
fi

echo "$successes $cost"
