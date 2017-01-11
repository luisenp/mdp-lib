#!/bin/bash

verbosity=1000
nsims=1000

echo "**** map4.track ****"
echo "++ vi"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map4.track --v=$verbosity --n=$nsims --debug --use-full --use-vi
echo "++ lao"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map4.track --v=$verbosity --n=$nsims --debug --use-full
echo "++ det"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map4.track --k=0 --v=$verbosity --n=$nsims --best-det --debug
echo "++ m02"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map4.track --k=0 --v=$verbosity --n=$nsims --best-m02 --debug
echo "++ m11"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map4.track --k=1 --v=$verbosity --n=$nsims --best-det --debug
echo "++ m12"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map4.track --k=1 --v=$verbosity --n=$nsims --best-m02 --debug

echo "**** map5.track ****"
echo "++ vi"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map5.track --v=$verbosity --n=$nsims --debug --use-full --use-vi
echo "++ lao"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map5.track --v=$verbosity --n=$nsims --debug --use-full
echo "++ det"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map5.track --k=0 --v=$verbosity --n=$nsims --best-det --debug
echo "++ m02"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map5.track --k=0 --v=$verbosity --n=$nsims --best-m02 --debug
echo "++ m11"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map5.track --k=1 --v=$verbosity --n=$nsims --best-det --debug
echo "++ m12"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map5.track --k=1 --v=$verbosity --n=$nsims --best-m02 --debug