#!/bin/bash

DOMAIN=gridworld
INPUT=../data/gws/map7.gw
# INPUT=../data/tracks/roads.track
NSIMS=10000
VERB=0

../simulmeta --domain=$DOMAIN --input=$INPUT --rule="assume1;nometa;optimal;qvimprov" \
  --nsims=$NSIMS --v=$VERB --steps_nop="1;2;3;4;5" --steps_action="1"