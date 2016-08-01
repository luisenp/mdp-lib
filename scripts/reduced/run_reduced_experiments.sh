#!/usr/bin/bash

PDDL_FOLDER=../../data/ppddl/ippc2008
PROBLEM=blocksworld

./create_all_determinizations_py -d $FOLDER/$PROBLEM/domain.pddl \
  -o /tmp/$PROBLEM
  
./setup_ff_template.py -p $FOLDER/$PROBLEM/p01.pddl -o /tmp/ff-template.pddl