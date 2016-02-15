#########################################################################
#                                VARIABLES                              #
#########################################################################


# Compilation flags and variables
CC = g++
CFLAGS = -std=c++11 -O3 -DATOM_STATES -pthread

# Variables for directories
ID = include
SD = src
TD = test
OD = obj
ID_UTIL = $(ID)/util
SD_UTIL = $(SD)/util
ID_SOLV = $(ID)/solvers
ID_PPDDL = $(ID)/ppddl
SD_SOLV = $(SD)/solvers
OD_SOLV = $(OD)/solvers
OD_PPDDL = $(OD)/ppddl
ID_REDUCED = $(ID)/reduced
SD_REDUCED = $(SD)/reduced
OD_REDUCED = $(OD)/reduced
OD_SOLV_MOBJ = $(OD)/solvers/mobj
ID_SOLV_MOBJ = $(ID)/solvers/mobj
SD_SOLV_MOBJ = $(SD)/solvers/mobj

ID_DOM = $(ID)/domains
SD_DOM = $(SD)/domains
ID_MODOM = $(ID)/mobj/domains
SD_MODOM = $(SD)/mobj/domains
SD_GW = $(SD_DOM)/gridworld
ID_GW = $(ID_DOM)/gridworld
SD_CTP = $(SD_DOM)/ctp
ID_CTP = $(ID_DOM)/ctp
SD_SAIL = $(SD_DOM)/sailing
ID_SAIL = $(ID_DOM)/sailing
SD_BT = $(SD_DOM)/binarytree
ID_BT = $(ID_DOM)/binarytree
SD_RACE = $(SD_DOM)/racetrack
ID_RACE = $(ID_DOM)/racetrack
OD_MODOM = $(OD)/domains/mobj

# Variables for include directives
INCLUDE_DOM = -I$(ID_GW) -I$(ID_CTP) -I$(ID_SAIL) -I$(ID_DOM) -I$(ID_RACE)
INCLUDE_CORE = -I$(ID_UTIL) -I$(ID)
INCLUDE_SOLVERS = -I$(ID_SOLV) -I$(ID_SOLV_MOBJ)
INCLUDE_PPDDL = -I$(ID_PPDDL) -I$(ID_PPDDL)/mini-gpt
INCLUDE = $(INCLUDE_DOM) $(INCLUDE_CORE) $(INCLUDE_SOLVERS)

# Variables for source/header files
I_H = $(ID)/*.h
S_CPP = $(SD)/*.cpp
SOLV_CPP = $(SD_SOLV)/*.cpp
SOLV_H = $(ID_SOLV)/*.h
MOSOLV_CPP = $(SD_SOLV_MOBJ)/*.cpp
MOSOLV_H = $(ID_SOLV_MOBJ)/*.h
UTIL_CPP = $(SD_UTIL)/*.cpp
UTIL_H = $(ID_UTIL)/*.h

GW_CPP = $(SD_GW)/*.cpp
GW_H = $(ID_GW)/*.h
CTP_CPP = $(SD_CTP)/*.cpp
CTP_H = $(ID_CTP)/*.h
SAIL_CPP = $(SD_SAIL)/*.cpp
SAIL_H = $(ID_SAIL)/*.h
BT_CPP = $(SD_BT)/*.cpp
BT_H = $(ID_BT)/*.h
RACE_CPP = $(SD_RACE)/*.cpp
RACE_H = $(ID_RACE)/*.h
DOM_CPP = $(GW_CPP) $(CTP_CPP) $(SAIL_CPP) $(RACE_CPP) $(SD_DOM)/*.cpp
DOM_H = $(GW_H) $(CTP_H) $(SAIL_H) $(RACE_H)

ALL_H = $(I_H) $(SOLV_H) $(MOSOLV_H) $(DOM_H) $(UTIL_H)
ALL_CPP = $(DOM_CPP) $(SOLV_CPP) $(MOSOLV_CPP) $(UTIL_CPP)

# Libraries
LIBS = lib/libmdp.a -Llib
LIBS_GUROBI = $(LIBS) -lgurobi60 -Llib

#########################################################################
#                                 TARGETS                               #
#########################################################################

# Compiles the core MDP-LIB library #
.PHONY: libmdp
libmdp:
	make $(OD)/core.a
	make $(OD)/solvers.a
	ar rvs libmdp.a $(OD)/core/*.o $(OD)/solvers/*.o
	mkdir -p lib
	mv libmdp.a lib

# Compiles the multi-objective solvers
$(OD)/mo-solvers.a: $(S_CPP) $(UTIL_CPP) $(I_H) $(UTIL_H) $(SOLV_CPP) $(SOLV_H) $(MOSOLV_H) $(MOSOLV_CPP)
	make $(OD)/core.a
	make $(OD)/solvers.a
	$(CC) $(CFLAGS) $(INCLUDE_CORE) $(INCLUDE_SOLVERS) -c $(MOSOLV_CPP)
	mkdir -p $(OD_SOLV_MOBJ)
	mv *.o $(OD_SOLV_MOBJ)
	mkdir -p lib
	ar rvs lib/libmdp_mosolvers.a $(OD_SOLV_MOBJ)/*.o

# Compiles the base (single-objective) solvers
$(OD)/solvers.a: $(S_CPP) $(UTIL_CPP) $(I_H) $(UTIL_H) $(SOLV_CPP) $(SOLV_H)
	make $(OD)/core.a
	$(CC) $(CFLAGS) $(INCLUDE_CORE) -c $(SOLV_CPP)
	mkdir -p $(OD_SOLV)
	mv *.o $(OD_SOLV)
	ar rvs $(OD)/solvers.a $(OD_SOLV)/*.o

# Compiles the core classes
$(OD)/core.a: $(S_CPP) $(UTIL_CPP) $(I_H) $(UTIL_H)
	$(CC) $(CFLAGS) $(INCLUDE_CORE) -c $(UTIL_CPP) $(S_CPP) $(UTIL_CPP)
	mkdir -p obj/core
	mv *.o obj/core
	ar rvs $(OD)/core.a $(OD)/core/*.o

# Compiles the multi-objective domains and test programs #
mobj: $(ALL_CPP) $(ALL_H)
	make $(OD)/mo-solvers.a
	make $(OD_MODOM)/airplane.a
	make $(OD_MODOM)/mo-racetrack.a
	make $(OD_MODOM)/mo-gw.a
	make $(OD_MODOM)/rawfile.a
	$(CC) $(CFLAGS) $(INCLUDE) -o testlexirace $(TD)/testLexiRace.cpp $(OD_MODOM)/*.o lib/libmdp_mosolvers.a $(LIBS_GUROBI)
	$(CC) $(CFLAGS) $(INCLUDE) -o testlexigw $(TD)/testLexiGW.cpp $(OD_MODOM)/*.o lib/libmdp_mosolvers.a $(LIBS_GUROBI)
	$(CC) $(CFLAGS) $(INCLUDE) -o testlexiraw $(TD)/testLexiRaw.cpp $(OD_MODOM)/*.o lib/libmdp_mosolvers.a $(LIBS_GUROBI)
	$(CC) $(CFLAGS) $(INCLUDE) -o testairplane $(TD)/testAirplane.cpp $(OD_MODOM)/*.o lib/libmdp_mosolvers.a $(LIBS_GUROBI)

# Compiles the airplane domain #
$(OD_MODOM)/airplane.a: $(ID_MODOM)/airplane/*.h $(SD_MODOM)/airplane/*.cpp
	rm -f *.o
	$(CC) $(CFLAGS) $(INCLUDE) -Iinclude/mobj/*.h -I$(ID_MODOM)/*.h \
	-I$(ID_MODOM)/airplane/*.h -c $(SD_MODOM)/airplane/*.cpp
	ar rvs $(OD_MODOM)/airplane.a *.o
	mkdir -p $(OD_MODOM)
	mv *.o $(OD_MODOM)

# Compiles the MO-Racetrack domain #
$(OD_MODOM)/mo-racetrack.a: $(RACE_CPP) $(RACE_H) $(ID_MODOM)/*rack*.h $(SD_MODOM)/*rack*.cpp
	rm -f *.o
	$(CC) $(CFLAGS) $(INCLUDE) -Iinclude/mobj/*.h -I$(ID_MODOM)/*.h \
	-I$(ID_MODOM)/*rack*/*.h -c $(SD_MODOM)/*rack*.cpp
	ar rvs $(OD_MODOM)/mo-racetrack.a *.o
	mkdir -p $(OD_MODOM)
	mv *.o $(OD_MODOM)

# Compiles the MO-Gridworld domain #
$(OD_MODOM)/mo-gw.a: $(ID_MODOM)/*.h $(SD_MODOM)/*.cpp $(GW_CPP) $(GW_H)
	rm -f *.o
	$(CC) $(CFLAGS) $(INCLUDE) -Iinclude/mobj/*.h -I$(ID_MODOM)/*.h \
	-I$(ID_MODOM)/*GridWorld*/*.h -c $(SD_MODOM)/*GridWorld*.cpp $(GW_CPP)
	ar rvs $(OD_MODOM)/mo-gw.a *.o
	mkdir -p $(OD_MODOM)
	mv *.o $(OD_MODOM)

# Compiles the Raw File domain #
$(OD_MODOM)/rawfile.a: $(ID_MODOM)/*Raw*.h $(SD_MODOM)/*Raw*.cpp
	rm -f *.o
	$(CC) $(CFLAGS) $(INCLUDE) -Iinclude/mobj/*.h -I$(ID_MODOM)/*.h \
	-I$(ID_MODOM)/*Raw*/*.h -c $(SD_MODOM)/*Raw*.cpp
	ar rvs $(OD_MODOM)/rawfile.a *.o
	mkdir -p $(OD_MODOM)
	mv *.o $(OD_MODOM)

# Compiles the concurrent planning test program #
conc: $(ALL_CPP) $(ALL_H)
	$(CC) $(CFLAGS) $(INCLUDE) -c $(DOM_CPP)
	mkdir -p test
	mv *.o test/
	$(CC) $(CFLAGS) $(INCLUDE) -o testconc.out $(TD)/testConc.cpp $(TD)/*.o $(LIBS)
	rm test/*.o

# Compiles the Racetrack domain test program #
race: $(I_H) $(RACE_H) $(RACE_CPP) $(S_CPP) libmdp
	$(CC) $(CFLAGS) -I$(ID_RACE) -I$(ID) -c $(RACE_CPP)
	mkdir -p test
	mv *.o test/
	$(CC) $(CFLAGS) $(INCLUDE) -o testrace.out $(TD)/testRace.cpp $(TD)/*.o $(LIBS)
	rm test/*.o

# Compiles the Sailing domain test program #
sail: $(I_H) $(SAIL_H) $(SAIL_CPP) $(S_CPP) libmdp
	$(CC) $(CFLAGS) -I$(ID_SAIL) -I$(ID) -c $(SAIL_CPP)
	mkdir -p test
	mv *.o test/
	$(CC) $(CFLAGS) $(INCLUDE) -o testsail.out $(TD)/testSail.cpp $(TD)/*.o $(LIBS)
	rm test/*.o

# Compiles the Canadian Traveler Problem domain test program #
ctp: $(CTP_CPP) $(SOLV_CPP) $(UTIL_CPP) $(I_H) $(SOLV_H) $(CTP_H) libmdp
	$(CC) $(CFLAGS) -I$(ID_CTP) $(INCLUDE_CORE) -c $(CTP_CPP)
	mkdir -p test
	mv *.o test/
	$(CC) $(CFLAGS) -I$(ID_CTP) $(INCLUDE_CORE) -o testctp.out $(TD)/testCTP.cpp $(TD)/*.o $(LIBS)
	rm test/*.o

# Compiles the Gridworld domain test program #
gw: $(GW_CPP) $(SOLV_CPP) $(UTIL_CPP) $(I_H) $(SOLV_H) $(GW_H) libmdp
	$(CC) $(CFLAGS) -I$(ID_GW) -I$(ID) -I$(ID_SOLV) -c $(GW_CPP)
	mkdir -p test
	mv *.o test/
	$(CC) $(CFLAGS) -I$(ID_GW) $(INCLUDE_CORE) -o testgw.out $(TD)/testGridWorld.cpp $(TD)/*.o $(LIBS)
	rm test/*.o

# Compiles a test program for a simple binary tree domain  #
b2t: $(BT_CPP) $(SOLV_CPP) $(UTIL_CPP) $(I_H) $(SOLV_H) $(BT_H) libmdp
	$(CC) $(CFLAGS) -I$(ID_BT) $(INCLUDE_CORE) -c $(BT_CPP)
	mkdir -p test
	mv *.o test/
	$(CC) $(CFLAGS) -I$(ID_BT) $(INCLUDE_CORE) -o testb2t.out $(TD)/testB2T.cpp $(TD)/*.o $(LIBS)
	rm test/*.o

# Compiles the mini-gpt library
minigpt:
	$(MAKE) -C include/ppddl/mini-gpt
	ar rvs lib/libminigpt.a include/ppddl/mini-gpt/*.o

# Compiles the PPDDL library
ppddl: libmdp src/ppddl/*.cpp include/ppddl/*.h minigpt
	$(CC) $(CFLAGS) -Iinclude -Iinclude/ppddl -Include/ppddl/mini-gpt -I$(ID_SOLV) -c src/ppddl/*.cpp
	mkdir -p test
	ar rvs lib/libmdp_ppddl.a *.o
	mkdir -p $(OD_PPDDL)
	mv *.o $(OD_PPDDL)
#	$(CC) $(CFLAGS) -Iinclude -I$(ID_SOLV) -I$(ID_UTIL) $(INCLUDE_PPDDL) -o testppddl.out $(TD)/testPPDDL.cpp include/ppddl/mini-gpt/heuristics.cc $(LIBS) lib/libminigpt.a lib/libmdp_ppddl.a

# Compiles the reduced model code
reduced: libmdp ppddl $(SD_REDUCED)/*.cpp $(ID_REDUCED)/*.h
	$(CC) $(CFLAGS) -Iinclude -I$(ID_REDUCED) -c $(SD_REDUCED)/*.cpp
	mkdir -p test
	ar rvs lib/libmdp_reduced.a *.o
	mkdir -p $(OD_REDUCED)
	mv *.o $(OD_REDUCED)
	$(CC) $(CFLAGS) $(INCLUDE) -c $(DOM_CPP)
	mkdir -p test
	mv *.o test/
	$(CC) $(CFLAGS) -I$(ID_REDUCED) $(INCLUDE_CORE) $(INCLUDE_PPDDL) -o testreduced.out $(TD)/reduced/testReduced.cpp test/*.o include/ppddl/mini-gpt/heuristics.cc $(LIBS) lib/libmdp_reduced.a lib/libminigpt.a lib/libmdp_ppddl.a

.PHONY: clean
clean:
	rm -f test/*.o
	rm -f *.o
	rm -f obj/*.a
	rm -f obj/*.o
	rm -f obj/core/*.o
	rm -f obj/domains/*.o
	rm -f obj/domains/*.a
	rm -f obj/domains/mobj/*
	rm -f obj/solvers/*.o
	rm -f obj/solvers/*.a
	rm -f obj/solvers/mobj/*
	rm -f include/ppddl/mini-gpt/*.o
