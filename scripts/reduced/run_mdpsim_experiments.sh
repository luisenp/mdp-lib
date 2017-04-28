#!/bin/bash

#
# Script used to run experiments with FF-Replan and RFF.
#
# The folder where the PPDDL files are stored. The contents should be 
# structured as follows:
#
# pddl_folder/domain-name-1/*.pddl
# pddl_folder/domain-name-2/*.pddl
#
# The PPDDL files must contain both domain and problem definitions and
# the problem name in the file must match the PPDDL filename.
#
pddl_folder=../../data/ppddl/ippc2008

# The domain name.
domain=triangle-tireworld

# The planner to use
planner=rff

# The determinization to use. The script assumes 
# determinizations are stored in folder /tmp/, with the following naming
# convention: <domain-name>_<determinization_name>_det.pddl.
determinization_name=mlo

test "$determinization_name" == 'mlo' && (./create_mlo_determinization.py -d $pddl_folder/$domain/domain.pddl -o /tmp/$domain)

for i in {01..10}; do
  problem=p$i
  
  # Setups the template problem for FF (removes PPDDL features not supported 
  # by FF and other cleanup)
  ./setup_ff_template.py -p $pddl_folder/$domain/$problem.pddl \
    -o /tmp/ff-template.pddl

  # Starts the planning server to connect to mdpsim
  ../../planserv_red.out --problem=$pddl_folder/$domain/$problem.pddl:$problem \
    --det_problem=${domain}_${determinization_name}_det.pddl \
    --det_descriptor=unused \
    --planner=$planner \
    --dir=/tmp --max-time=1200 --debug &> planserv_log.txt &

  # Starts the mdpsim server
  ../../../mdpsim-2.2/mdpsim --port=2323 --time-limit=1200000 --round-limit=50 \
    --turn-limit=2500 $pddl_folder/$domain/$problem.pddl &> sever_log.txt &

  # This might not be necessary, but just in case
  sleep 5

  # Starts the mdpsim client
  ../../../mdpsim-2.2/mdpclient --host=localhost --port=2323 \
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
done
