./planner -x 100 -r 100 -v 100 -h "atom-min-1-backward" 0:2412 ../examples/tile8-manhattan.pddl i1 2> /dev/null > manhattan-8
grep "state-value" manhattan-8 | awk '{ ++i; printf "r%d %s\n", i, $2; }' | sort > tmp-manh-8

./planner -x 100 -r 100 -v 100 -h "atom-min-1-backward" 0:2412 ../examples/tile8-sum-pair.pddl i1 2> /dev/null > sum-pair-8
grep "state-value" sum-pair-8 | awk '{ ++i; printf "r%d %s\n", i, $2; }' | sort > tmp-pair-8

grep init manhattan-8 | nawk 'BEGIN { FS="[\ \(\)]+"; } { for(i=2;i<NF;++i) print $i; }' | nawk 'BEGIN { FS="_"; i=0; } /:init/ { if(i>0) { for(i=0;i<9;++i) printf "%d ",line[i]; printf "\n"; } for(i=0;i<9;++i) line[i]=0; } $0!~/:init/ { ++i; if( $2 != "blank" ) { tile=substr($2,2); row=substr($3,2); col=substr($4,2); pos=3*(row-1)+(col-1); line[pos]=tile; } } END { for(i=0;i<9;++i) printf "%d ",line[i]; printf "\n"; }' > puzzle-instances-8

cat puzzle-instances-8 | nawk '{ system( "../../n-puzzle/puzzle 3 \"" $0 "\"" ); }' | grep last | awk '{ ++i; printf "r%d %s\n",i,$4; }' | sort > tmp-sol-8

join tmp-sol tmp-pair | join - tmp-manh | awk 'BEGIN {printf "(define diff1 (list ";} { printf "%d ",$2-$3; } END { printf "))\n\n";}' > diff-8.scm

join tmp-sol tmp-pair | join - tmp-manh | awk 'BEGIN {printf "(define diff2 (list ";} { printf "%d ",$2-$4; } END { printf "))\n\n";}' >> diff-8.scm

join tmp-sol tmp-pair | join - tmp-manh | awk 'BEGIN {printf "(define diff3 (list ";} { printf "%d ",$3-$4; } END { printf "))\n\n";}' >> diff-8.scm






#./planner -x 1000 -r 100 -v 100 -h "atom-min-1-backward" 0:2411 ../examples/tile15-manhattan.pddl i1 2> /dev/null > manhattan-15
#grep "state-value" manhattan-15 | awk '{ ++i; printf "r%d %s\n", i, $2; }' | sort > tmp-manh-15


#./planner -x 1000 -r 100 -v 100 -h "atom-min-1-backward" 0:2411 ../examples/tile15-sum-pair.pddl i1 2> /dev/null > sum-pair-15
#grep "state-value" sum-pair-15 | awk '{ ++i; printf "r%d %s\n", i, $2; }' | sort > tmp-pair-15
