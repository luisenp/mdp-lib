#!/bin/bash

for i in `seq 1 112`;
do
  ./testreduced.out --domain=ppddl --problem=data/ppddl/ippc2008/triangle-tireworld/p02.pddl:triangle-tire-2 --k=0 --v=0 --stop=$i
done
