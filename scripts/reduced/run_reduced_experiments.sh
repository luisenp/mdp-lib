#!/bin/bash

PDDL_FOLDER=../../data/ppddl/ippc2008
DOMAIN=blocksworld
PROBLEM=p01

./create_all_determinizations.py -d $PDDL_FOLDER/$DOMAIN/domain.pddl \
  -o /tmp/$DOMAIN
  
NUM_DET=`ls /tmp/${DOMAIN}_det*.pddl -l | wc -l`

for i in `seq 0 $NUM_DET`; do
  echo $i
done

sleep 1

DET_IDX=1

./setup_ff_template.py -p $PDDL_FOLDER/$DOMAIN/$PROBLEM.pddl \
  -o /tmp/ff-template.pddl

../../planserv_red.out --problem=$PDDL_FOLDER/$DOMAIN/$PROBLEM.pddl:$PROBLEM \
  --det_problem=${DOMAIN}_det${DET_IDX}.pddl \
  --det_descriptor=/tmp/${DOMAIN}_det${DET_IDX}.desc \
  --dir=/tmp --k=0 &

../../../mdpsim-2.2/mdpsim --port=2323 --time-limit=1200000 --round-limit=50 \
  --turn-limit=2500 $PDDL_FOLDER/$DOMAIN/$PROBLEM.pddl &

sleep 2

../../../mdpsim-2.2/mdpclient --host=localhost --port=2323 \
  $PDDL_FOLDER/$DOMAIN/$PROBLEM.pddl &> log.txt

kill $(ps aux | grep '[p]lanserv' | awk '{print $2}')
kill $(ps aux | grep '[l]t-mdpsim' | awk '{print $2}')
kill $(ps aux | grep '[l]t-mdpclient' | awk '{print $2}')
rm -f last_id