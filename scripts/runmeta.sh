#!/bin/bash

DOMAIN=racetrack
# INPUT=../data/gws/map7.gw
INPUT=../data/tracks/roads.track
NSIMS=10000
VERB=0

../simulmeta --domain=$DOMAIN --input=$INPUT --meta=assume1 --nsims=$NSIMS --v=$VERB --steps_range=10
../simulmeta --domain=$DOMAIN --input=$INPUT --meta=nometa --nsims=$NSIMS --v=$VERB --steps_range=10
../simulmeta --domain=$DOMAIN --input=$INPUT --meta=optimal --nsims=$NSIMS --v=$VERB --steps_range=10
../simulmeta --domain=$DOMAIN --input=$INPUT --meta=qvimprov --nsims=$NSIMS --v=$VERB --steps_range=10