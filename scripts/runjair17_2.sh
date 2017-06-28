#!/bin/bash

nsims=100
verbosity=100

echo "++vi"
./testreduced.out --domain=sailing --sailing-size=50 --sailing-goal=49 --use-full --use-vi --debug --n=$nsims --v=$verbosity
echo "++lao*"
./testreduced.out --domain=sailing --sailing-size=50 --sailing-goal=49 --use-full --debug --n=$nsims --v=$verbosity
echo "++det"
./testreduced.out --domain=sailing --sailing-size=50 --sailing-goal=49 --best-det-sailing-greedy --debug --k=0 --n=$nsims --v=$verbosity
echo "++m02"
./testreduced.out --domain=sailing --sailing-size=50 --sailing-goal=49 --best-m02-sailing-greedy --debug --k=0 --n=$nsims --v=$verbosity
echo "++m11"
./testreduced.out --domain=sailing --sailing-size=50 --sailing-goal=49 --best-det-sailing-greedy --debug --k=1 --n=$nsims --v=$verbosity
echo "++m12"
./testreduced.out --domain=sailing --sailing-size=50 --sailing-goal=49 --best-m02-sailing-greedy --debug --k=1 --n=$nsims --v=$verbosity
