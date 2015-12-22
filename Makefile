#########################################################################
#                                VARIABLES                              #
#########################################################################


# Compilation flags and variables
CC = g++
CFLAGS = -std=c++11 -g -DATOM_STATES -pthread

# Variables for directories
ID = include
SD = src
TD = test
OD = obj
ID_UTIL = $(ID)/util
SD_UTIL = $(SD)/util
ID_SOLV = $(ID)/solvers
SD_SOLV = $(SD)/solvers
OD_SOLV = $(OD)/solvers
OD_SOLV_MOBJ = $(OD)/solvers/mobj
OD_SOLV_META = $(OD)/solvers/metareasoning
ID_SOLV_MOBJ = $(ID)/solvers/mobj
ID_SOLV_META = $(ID)/solvers/metareasoning
SD_SOLV_MOBJ = $(SD)/solvers/mobj
SD_SOLV_META = $(SD)/solvers/metareasoning


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
INCLUDE_SOLVERS = -I$(ID_SOLV) -I$(ID_SOLV_MOBJ) -I$(ID_SOLV_META)
INCLUDE = $(INCLUDE_DOM) $(INCLUDE_CORE) $(INCLUDE_SOLVERS)

# Variables for source/header files
I_H = $(ID)/*.h
S_CPP = $(SD)/*.cpp
SOLV_CPP = $(SD_SOLV)/*.cpp
SOLV_H = $(ID_SOLV)/*.h
MOSOLV_CPP = $(SD_SOLV_MOBJ)/*.cpp
META_CPP = $(SD_SOLV_META)/*.cpp
MOSOLV_H = $(ID_SOLV_MOBJ)/*.h
META_H = $(ID_SOLV_META)/*.h
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

ALL_H = $(I_H) $(SOLV_H) $(MOSOLV_H) $(DOM_H) $(UTIL_H) $(META_H)
ALL_CPP = $(DOM_CPP) $(SOLV_CPP) $(MOSOLV_CPP) $(UTIL_CPP) $(META_CPP)

# Libraries
LIBS = lib/libmdp.a -Llib
LIBS_MOBJ = $(LIBS) -lgurobi_c++ -lgurobi60

#########################################################################
#                                 TARGETS                               #
#########################################################################

# Compiles the core MDP-LIB library #
.PHONY: libmdp
libmdp:
	mkdir -p $(OD)/core
	mkdir -p $(OD)/solvers
	mkdir -p lib
	make $(OD)/core.a
	make $(OD)/solvers.a
	ar rvs libmdp.a $(OD)/core/*.o $(OD)/solvers/*.o
	mv libmdp.a lib

# Compiles the base (single-objective) solvers
$(OD)/solvers.a: $(S_CPP) $(UTIL_CPP) $(I_H) $(UTIL_H) $(SOLV_CPP) $(SOLV_H)
	make $(OD)/core.a
	$(CC) $(CFLAGS) $(INCLUDE_CORE) -c $(SOLV_CPP)
	mv *.o $(OD_SOLV)
	ar rvs $(OD)/solvers.a $(OD_SOLV)/*.o

# Compiles the multi-objective solvers
$(OD)/mo-solvers.a: $(S_CPP) $(UTIL_CPP) $(I_H) $(UTIL_H) $(SOLV_CPP) $(SOLV_H) $(MOSOLV_H) $(MOSOLV_CPP)
	make libmdp
	$(CC) $(CFLAGS) $(INCLUDE_CORE) $(INCLUDE_SOLVERS) -c $(MOSOLV_CPP)
	mv *.o $(OD_SOLV_MOBJ)
	ar rvs obj/mo-solvers.a $(OD_SOLV_MOBJ)/*.o
	mv obj/mo-solvers.a lib

# Compiles the metareasoning code
$(OD)/meta.a: $(S_CPP) $(UTIL_CPP) $(I_H) $(UTIL_H) $(SOLV_CPP) $(SOLV_H) $(META_H) $(META_CPP)
	make libmdp
	$(CC) $(CFLAGS) $(INCLUDE_CORE) $(INCLUDE_SOLVERS) -c $(META_CPP)
	mv *.o $(OD_SOLV_META)
	ar rvs obj/meta.a $(OD_SOLV_META)/*.o
	mv obj/meta.a lib

# Compiles the core classes
$(OD)/core.a: $(S_CPP) $(UTIL_CPP) $(I_H) $(UTIL_H)
	$(CC) $(CFLAGS) $(INCLUDE_CORE) -c $(UTIL_CPP) $(S_CPP) $(UTIL_CPP)
	mv *.o obj/core
	ar rvs $(OD)/core.a $(OD)/core/*.o

# Compiles the multiobjective domains and test programs #
mobj: $(ALL_CPP) $(ALL_H)
	make $(OD)/mo-solvers.a
	make $(OD_MODOM)/airplane.a
	make $(OD_MODOM)/mo-racetrack.a
	make $(OD_MODOM)/mo-gw.a
	make $(OD_MODOM)/rawfile.a
	$(CC) $(CFLAGS) $(INCLUDE) -o testlexirace $(TD)/mobj/testLexiRace.cpp $(OD_MODOM)/*.o lib/mo-solvers.a $(LIBS)
	$(CC) $(CFLAGS) $(INCLUDE) -o testlexigw $(TD)/mobj/testLexiGW.cpp $(OD_MODOM)/*.o lib/mo-solvers.a $(LIBS)
	$(CC) $(CFLAGS) $(INCLUDE) -o testlexiraw $(TD)/mobj/testLexiRaw.cpp $(OD_MODOM)/*.o lib/mo-solvers.a $(LIBS)
	$(CC) $(CFLAGS) $(INCLUDE) -o testairplane $(TD)/mobj/testAirplane.cpp $(OD_MODOM)/*.o lib/mo-solvers.a $(LIBS)

# Compiles the airplane domain #
$(OD_MODOM)/airplane.a: $(ID_MODOM)/airplane/*.h $(SD_MODOM)/airplane/*.cpp
	rm -f *.o
	$(CC) $(CFLAGS) $(INCLUDE) -Iinclude/mobj/*.h -I$(ID_MODOM)/*.h \
	-I$(ID_MODOM)/airplane/*.h -c $(SD_MODOM)/airplane/*.cpp
	ar rvs $(OD_MODOM)/airplane.a *.o
	mv *.o $(OD_MODOM)

# Compiles the MO-Racetrack domain #
$(OD_MODOM)/mo-racetrack.a: $(RACE_CPP) $(RACE_H) $(ID_MODOM)/*rack*.h $(SD_MODOM)/*rack*.cpp
	rm -f *.o
	$(CC) $(CFLAGS) $(INCLUDE) -Iinclude/mobj/*.h -I$(ID_MODOM)/*.h \
	-I$(ID_MODOM)/*rack*/*.h -c $(SD_MODOM)/*rack*.cpp
	ar rvs $(OD_MODOM)/mo-racetrack.a *.o
	mv *.o $(OD_MODOM)

# Compiles the MO-Gridworld domain #
$(OD_MODOM)/mo-gw.a: $(ID_MODOM)/*.h $(SD_MODOM)/*.cpp $(GW_CPP) $(GW_H) Makefile
	rm -f *.o
	$(CC) $(CFLAGS) $(INCLUDE) -Iinclude/mobj/*.h -I$(ID_MODOM)/*.h \
	-I$(ID_MODOM)/*GridWorld*/*.h -c $(SD_MODOM)/*GridWorld*.cpp $(GW_CPP)
	ar rvs $(OD_MODOM)/mo-gw.a *.o
	mv *.o $(OD_MODOM)

# Compiles the Raw File domain #
$(OD_MODOM)/rawfile.a: $(ID_MODOM)/*Raw*.h $(SD_MODOM)/*Raw*.cpp
	rm -f *.o
	$(CC) $(CFLAGS) $(INCLUDE) -Iinclude/mobj/*.h -I$(ID_MODOM)/*.h \
	-I$(ID_MODOM)/*Raw*/*.h -c $(SD_MODOM)/*Raw*.cpp
	ar rvs $(OD_MODOM)/rawfile.a *.o
	mv *.o $(OD_MODOM)

# Compiles the concurrent planning test program #
conc: $(ALL_CPP) $(ALL_H)
	$(CC) $(CFLAGS) $(INCLUDE) -c $(DOM_CPP)
	mv *.o test/
	$(CC) $(CFLAGS) $(INCLUDE) -o testconc $(TD)/testConc.cpp $(TD)/*.o $(LIBS)
	rm test/*.o

# Compiles the Racetrack domain test program #
race: $(I_H) $(RACE_H) $(RACE_CPP) $(S_CPP)
	$(CC) $(CFLAGS) -I$(ID_RACE) -I$(ID) -c $(RACE_CPP)
	mv *.o test/
	$(CC) $(CFLAGS) $(INCLUDE) -o testrace $(TD)/testRace.cpp $(TD)/*.o $(LIBS)
	rm test/*.o

# Compiles the Metareasoning algorithm test program #
meta: $(I_H) $(RACE_H) $(RACE_CPP) $(S_CPP) $(GW_CPP) $(OD)/meta.a
	make $(OD)/meta.a
	$(CC) $(CFLAGS) -I$(ID_RACE) -I$(ID) -c $(RACE_CPP) $(GW_CPP)
	mv *.o test/
	$(CC) $(CFLAGS) $(INCLUDE) -o testmeta $(TD)/testMetareasoning.cpp $(TD)/*.o lib/meta.a $(LIBS)
	$(CC) $(CFLAGS) $(INCLUDE) -o simulmeta $(TD)/simulateMetareasoning.cpp $(TD)/*.o lib/meta.a $(LIBS)
	rm test/*.o

# Compiles the Sailing domain test program #
sail: $(I_H) $(SAIL_H) $(SAIL_CPP) $(S_CPP)
	$(CC) $(CFLAGS) -I$(ID_SAIL) -I$(ID) -c $(SAIL_CPP)
	mv *.o test/
	$(CC) $(CFLAGS) $(INCLUDE) -o testsail $(TD)/testSail.cpp $(TD)/*.o $(LIBS)
	rm test/*.o

# Compiles the Canadian Traveler Problem domain test program #
ctp: $(CTP_CPP) $(SOLV_CPP) $(UTIL_CPP) $(I_H) $(SOLV_H) $(CTP_H)
	$(CC) $(CFLAGS) -I$(ID_CTP) $(INCLUDE_CORE) -c $(CTP_CPP)
	mv *.o test/
	$(CC) $(CFLAGS) -I$(ID_CTP) $(INCLUDE_CORE) -o testctp $(TD)/testCTP.cpp $(TD)/*.o $(LIBS)
	rm test/*.o

# Compiles the Gridworld domain test program #
gw: $(GW_CPP) $(SOLV_CPP) $(UTIL_CPP) $(I_H) $(SOLV_H) $(GW_H)
	$(CC) $(CFLAGS) -I$(ID_GW) -I$(ID) -I$(ID_SOLV) -c $(GW_CPP)
	mv *.o test/
	$(CC) $(CFLAGS) -I$(ID_GW) $(INCLUDE_CORE) -o testgw $(TD)/testGridWorld.cpp $(TD)/*.o $(LIBS)
	rm test/*.o

# Compiles a test program for a simple binary tree domain  #
b2t: $(BT_CPP) $(SOLV_CPP) $(UTIL_CPP) $(I_H) $(SOLV_H) $(BT_H)
	$(CC) $(CFLAGS) -I$(ID_BT) $(INCLUDE_CORE) -c $(BT_CPP)
	mv *.o test/
	$(CC) $(CFLAGS) -I$(ID_BT) $(INCLUDE_CORE) -o testb2t $(TD)/testB2T.cpp $(TD)/*.o $(LIBS)
	rm test/*.o

# Compiles the PPDDL library
ppddl: src/ppddl/*.cpp $(I_H) include/ppddl/*.h include/ppddl/mini-gpt/*.h $(SOLV_CPP) $(UTIL_CPP)
	$(CC) $(CFLAGS) -Iinclude -Iinclude/ppddl -Include/ppddl/mini-gpt -I$(ID_SOLV) -c src/ppddl/*.cpp src/*.cpp $(SOLV_CPP) $(UTIL_CPP)
	mv *.o test/
	$(CC) $(CFLAGS) -Iinclude -I$(ID_SOLV) -I$(ID_UTIL) -o testppddl test/testPPDDL.cpp test/*.o $(LIBS) lib/libminigpt.a

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
	rm -f lib/libmdp.a
	rm -f lib/meta.a
	rm -f lib/mo-solvers.a
