#!/bin/bash

for i in `seq -f %f 0.7 0.02 1.0`;
do
    ./testlexirace data/tracks/lexi/track2.track lao 0.0 0 10000 1 $i
done
