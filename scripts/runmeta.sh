#!/bin/bash

GRIDS=("../data/gws/map3.gw" "../data/gws/map4.gw" "../data/gws/map5.gw" "../data/gws/map7.gw")
RACETRACKS=("../data/tracks/roads.track")
NSIMS=10000
VERB=0

for INPUT in "${RACETRACKS[@]}"
do
  echo $INPUT
  ../simulmeta --domain=racetrack --input=$INPUT \
    --rule="assume1;nometa;optimal;qvimprov;multnop" \
    --nsims=$NSIMS --v=$VERB --steps_nop="1;2;3;4;5;6;7;8;9;10" \
    --steps_action="1"
done
  