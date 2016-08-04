#!/bin/bash

pddl_folder=$1
domain=$2
problem=$3
determinization_index=$4

echo $1
echo $2
echo $3
echo $4

# Setups the template problem for FF (removes PPDDL features not supported 
# by FF and other cleanup)
./setup_ff_template.py -p $pddl_folder/$domain/$problem.pddl \
  -o /tmp/ff-template.pddl

# Starts the planning server to connect to mdpsim
../../planserv_red.out --problem=$pddl_folder/$domain/$problem.pddl:$problem \
  --det_problem=${domain}_det${determinization_index}.pddl \
  --det_descriptor=/tmp/${domain}_det${determinization_index}.desc \
  --dir=/tmp --k=0 &

# Starts the mdpsim server
../../../mdpsim-2.2/mdpsim --port=2323 --time-limit=1200000 --round-limit=50 \
  --turn-limit=2500 $pddl_folder/$domain/$problem.pddl &

# This might not be necessary, but just in case
sleep 1

# Starts the mdpsim client
../../../mdpsim-2.2/mdpclient --host=localhost --port=2323 \
  $pddl_folder/$domain/$problem.pddl &> log.txt

# Kill the planning and mdpsim servers
kill $(ps aux | grep '[p]lanserv' | awk '{print $2}')
kill $(ps aux | grep '[l]t-mdpsim' | awk '{print $2}')
rm -f last_id

# Extract the number of successes and turns (cost) resulting from this 
# determinization
successes=`grep -o "<successes>[0-9.]*" log.txt | grep -o "[0-9.]*"`
cost=`grep -o "<turn-average>[0-9.]*" log.txt | grep -o "[0-9.]*"`
if [ -z "$cost" ]; then
  cost=10000.0
fi

echo "$successes $cost"