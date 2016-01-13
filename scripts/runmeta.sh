#!/bin/bash

GRID=../data/gws/map7.gw
NSIMS=1
VERB=0

# ../simulmeta --grid=$GRID --meta=assume1 --nsims=$NSIMS --v=$VERB --steps_range=10
# ../simulmeta --grid=$GRID --meta=assume2 --nsims=$NSIMS --v=$VERB --steps_range=10
# ../simulmeta --grid=$GRID --meta=change --nsims=$NSIMS --v=$VERB --steps_range=10

../simulmeta --grid=$GRID --meta=assume1 --nsims=$NSIMS --v=$VERB --steps_range=10
../simulmeta --grid=$GRID --meta=nometa --nsims=$NSIMS --v=$VERB --steps_range=10
../simulmeta --grid=$GRID --meta=optimal --nsims=$NSIMS --v=$VERB --steps_range=10
../simulmeta --grid=$GRID --meta=qvimprov --nsims=$NSIMS --v=$VERB --steps_range=10