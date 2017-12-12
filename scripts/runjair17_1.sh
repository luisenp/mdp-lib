#!/bin/bash

verbosity=0
nsims=100
mdsval=-1
kappaval=1

echo "**** map3.track ****"
echo "++ vi"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map3.track --v=$verbosity --n=$nsims --debug --mds=$mdsval --kappa=$kappaval --use-full --use-vi
echo "++ lao"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map3.track --v=$verbosity --n=$nsims --debug --mds=$mdsval --kappa=$kappaval --use-full
echo "++ det"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map3.track --k=0 --v=$verbosity --n=$nsims --best-det-racetrack-greedy --debug --mds=$mdsval --kappa=$kappaval
echo "++ m02"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map3.track --k=0 --v=$verbosity --n=$nsims --best-m02-racetrack-greedy --debug --mds=$mdsval --kappa=$kappaval
echo "++ m11"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map3.track --k=1 --v=$verbosity --n=$nsims --best-det-racetrack-greedy --debug --mds=$mdsval --kappa=$kappaval
echo "++ m12"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map3.track --k=1 --v=$verbosity --n=$nsims --best-m02-racetrack-greedy --debug --mds=$mdsval --kappa=$kappaval

echo "**** map4.track ****"
echo "++ vi"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map4.track --v=$verbosity --n=$nsims --debug --mds=$mdsval --kappa=$kappaval --use-full --use-vi
echo "++ lao"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map4.track --v=$verbosity --n=$nsims --debug --mds=$mdsval --kappa=$kappaval --use-full
echo "++ det"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map4.track --k=0 --v=$verbosity --n=$nsims --best-det-racetrack-greedy --debug --mds=$mdsval --kappa=$kappaval
echo "++ m02"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map4.track --k=0 --v=$verbosity --n=$nsims --best-m02-racetrack-greedy --debug --mds=$mdsval --kappa=$kappaval
echo "++ m11"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map4.track --k=1 --v=$verbosity --n=$nsims --best-det-racetrack-greedy --debug --mds=$mdsval --kappa=$kappaval
echo "++ m12"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map4.track --k=1 --v=$verbosity --n=$nsims --best-m02-racetrack-greedy --debug --mds=$mdsval --kappa=$kappaval

echo "**** map5.track ****"
echo "++ vi"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map5.track --v=$verbosity --n=$nsims --debug --mds=$mdsval --kappa=$kappaval --use-full --use-vi
echo "++ lao"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map5.track --v=$verbosity --n=$nsims --debug --mds=$mdsval --kappa=$kappaval --use-full
echo "++ det"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map5.track --k=0 --v=$verbosity --n=$nsims --best-det-racetrack-greedy --debug --mds=$mdsval --kappa=$kappaval
echo "++ m02"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map5.track --k=0 --v=$verbosity --n=$nsims --best-m02-racetrack-greedy --debug --mds=$mdsval --kappa=$kappaval
echo "++ m11"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map5.track --k=1 --v=$verbosity --n=$nsims --best-det-racetrack-greedy --debug --mds=$mdsval --kappa=$kappaval
echo "++ m12"
../testreduced.out --domain=racetrack --problem=../data/tracks/roads-huge/map5.track --k=1 --v=$verbosity --n=$nsims --best-m02-racetrack-greedy --debug --mds=$mdsval --kappa=$kappaval