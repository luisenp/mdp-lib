#!/bin/bash

DOMAIN=gridworld
INPUT=../data/gws/map5.gw
# INPUT=../data/tracks/roads.track
NSIMS=1
VERB=10000

# ../simulmeta --domain=$DOMAIN --input=$INPUT --rule="assume1;nometa;optimal;qvimprov" --nsims=$NSIMS --v=$VERB --steps_range=10
../simulmeta --domain=$DOMAIN --input=$INPUT --rule="assume1" --nsims=$NSIMS --v=$VERB --steps_range=1