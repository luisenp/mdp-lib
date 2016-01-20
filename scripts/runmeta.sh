#!/bin/bash

DOMAIN=gridworld
INPUT=../data/gws/map4.gw
# INPUT=../data/tracks/roads.track
NSIMS=10000
VERB=0
 
# ../simulmeta --domain=$DOMAIN --input=$INPUT --rule="assume1;nometa;optimal;qvimprov;multnop" \
#   --nsims=$NSIMS --v=$VERB --steps_nop="1;2;3;4;5;6;7;8;9;10" \
#   --steps_action="1" --same_cost_nop
#   --steps_action="1;2;3;4;5;6;7;8;9;10" --same_cost_nop
  
# ../simulmeta --domain=$DOMAIN --input=$INPUT --rule="multnop" \
#   --nsims=$NSIMS --v=$VERB --steps_nop="1;2;3;4;5;6;7;8;9;10" \
#   --steps_action="1;2;3;4;5;6;7;8;9;10"

../simulmeta --domain=$DOMAIN --input=$INPUT --rule="optimal" \
  --nsims=$NSIMS --v=$VERB --steps_nop="1;2;3;4;5;6;7;8;9;10" \
  --steps_action="1" --same_cost_nop
  --steps_action="1;2;3;4;5;6;7;8;9;10" --same_cost_nop