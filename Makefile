#########################################################################
#                                VARIABLES                              #
#########################################################################


# Compilation flags and variables
CC = g++
CFLAGS = -std=c++11 -O3 -DATOM_STATES -DNDEBUG -pthread

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

ID_DOMAINS = $(ID)/domains
SD_DOMAINS = $(SD)/domains
OD_DOMAINS = $(OD)/domains
ID_MOBJ_DOMAINS = $(ID)/mobj/domains
SD_MOBJ_DOMAINS = $(SD)/mobj/domains
OD_MOBJ_DOMAINS = $(OD)/domains/mobj
SD_GW = $(SD_DOMAINS)/gridworld
ID_GW = $(ID_DOMAINS)/gridworld
SD_CTP = $(SD_DOMAINS)/ctp
ID_CTP = $(ID_DOMAINS)/ctp
SD_SAIL = $(SD_DOMAINS)/sailing
ID_SAIL = $(ID_DOMAINS)/sailing
SD_BT = $(SD_DOMAINS)/binarytree
ID_BT = $(ID_DOMAINS)/binarytree
SD_RACE = $(SD_DOMAINS)/racetrack
ID_RACE = $(ID_DOMAINS)/racetrack
SD_BORDER = $(SD_DOMAINS)/borderexit
ID_BORDER = $(ID_DOMAINS)/borderexit

# Variables for include directives
INCLUDE_DOM = -I$(ID_GW) -I$(ID_CTP) -I$(ID_SAIL) -I$(ID_DOMAINS) -I$(ID_RACE)
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
BORDER_CPP = $(SD_BORDER)/*.cpp
BORDER_H = $(ID_BORDER)/*.h
DOM_CPP = $(GW_CPP) $(CTP_CPP) $(SAIL_CPP) $(RACE_CPP) $(BORDER_CPP) $(SD_DOMAINS)/*.cpp
DOM_H = $(GW_H) $(CTP_H) $(SAIL_H) $(RACE_H) $(BORDER_H)

ALL_H = $(I_H) $(SOLV_H) $(MOSOLV_H) $(DOM_H) $(UTIL_H)
ALL_CPP = $(DOM_CPP) $(SOLV_CPP) $(MOSOLV_CPP) $(UTIL_CPP)

# Libraries
LIBS = lib/libmdp.a lib/libmdp_domains.a -Llib
LIBS_GUROBI = $(LIBS) -lgurobi60 -Llib

#########################################################################
#                                 TARGETS                               #
#########################################################################

# Compiles the core MDP-LIB library #
libmdp: lib/libmdp.a
lib/libmdp.a: $(OD)/core.a $(OD)/solvers.a
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
$(OD)/solvers.a: $(S_CPP) $(UTIL_CPP) $(I_H) $(UTIL_H) $(SOLV_CPP) $(SOLV_H) $(ID_DOMAINS)/*.h $(SD_DOMAINS)/*.cpp
	make $(OD)/core.a
	$(CC) $(CFLAGS) $(INCLUDE_CORE) $(ID_DOMAINS)/*.h -c $(SOLV_CPP) \
		$(SD_DOMAINS)/*.cpp
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
	make $(OD_MOBJ_DOMAINS)/airplane.a
	make $(OD_MOBJ_DOMAINS)/mo-racetrack.a
	make $(OD_MOBJ_DOMAINS)/mo-gw.a
	make $(OD_MOBJ_DOMAINS)/rawfile.a
	$(CC) $(CFLAGS) $(INCLUDE) -o testlexirace $(TD)/testLexiRace.cpp $(OD_MOBJ_DOMAINS)/*.o lib/libmdp_mosolvers.a $(LIBS_GUROBI)
	$(CC) $(CFLAGS) $(INCLUDE) -o testlexigw $(TD)/testLexiGW.cpp $(OD_MOBJ_DOMAINS)/*.o lib/libmdp_mosolvers.a $(LIBS_GUROBI)
	$(CC) $(CFLAGS) $(INCLUDE) -o testlexiraw $(TD)/testLexiRaw.cpp $(OD_MOBJ_DOMAINS)/*.o lib/libmdp_mosolvers.a $(LIBS_GUROBI)
	$(CC) $(CFLAGS) $(INCLUDE) -o testairplane $(TD)/testAirplane.cpp $(OD_MOBJ_DOMAINS)/*.o lib/libmdp_mosolvers.a $(LIBS_GUROBI)

# Compiles the airplane domain #
$(OD_MOBJ_DOMAINS)/airplane.a: $(ID_MOBJ_DOMAINS)/airplane/*.h $(SD_MOBJ_DOMAINS)/airplane/*.cpp
	rm -f *.o
	$(CC) $(CFLAGS) $(INCLUDE) -Iinclude/mobj/*.h -I$(ID_MOBJ_DOMAINS)/*.h \
	-I$(ID_MOBJ_DOMAINS)/airplane/*.h -c $(SD_MOBJ_DOMAINS)/airplane/*.cpp
	ar rvs $(OD_MOBJ_DOMAINS)/airplane.a *.o
	mkdir -p $(OD_MOBJ_DOMAINS)
	mv *.o $(OD_MOBJ_DOMAINS)

# Compiles the MO-Racetrack domain #
$(OD_MOBJ_DOMAINS)/mo-racetrack.a: $(RACE_CPP) $(RACE_H) $(ID_MOBJ_DOMAINS)/*rack*.h $(SD_MOBJ_DOMAINS)/*rack*.cpp
	rm -f *.o
	$(CC) $(CFLAGS) $(INCLUDE) -Iinclude/mobj/*.h -I$(ID_MOBJ_DOMAINS)/*.h \
	-I$(ID_MOBJ_DOMAINS)/*rack*/*.h -c $(SD_MOBJ_DOMAINS)/*rack*.cpp
	ar rvs $(OD_MOBJ_DOMAINS)/mo-racetrack.a *.o
	mkdir -p $(OD_MOBJ_DOMAINS)
	mv *.o $(OD_MOBJ_DOMAINS)

# Compiles the MO-Gridworld domain #
$(OD_MOBJ_DOMAINS)/mo-gw.a: $(ID_MOBJ_DOMAINS)/*.h $(SD_MOBJ_DOMAINS)/*.cpp $(GW_CPP) $(GW_H)
	rm -f *.o
	$(CC) $(CFLAGS) $(INCLUDE) -Iinclude/mobj/*.h -I$(ID_MOBJ_DOMAINS)/*.h \
	-I$(ID_MOBJ_DOMAINS)/*GridWorld*/*.h -c $(SD_MOBJ_DOMAINS)/*GridWorld*.cpp $(GW_CPP)
	ar rvs $(OD_MOBJ_DOMAINS)/mo-gw.a *.o
	mkdir -p $(OD_MOBJ_DOMAINS)
	mv *.o $(OD_MOBJ_DOMAINS)

# Compiles the Raw File domain #
$(OD_MOBJ_DOMAINS)/rawfile.a: $(ID_MOBJ_DOMAINS)/*Raw*.h $(SD_MOBJ_DOMAINS)/*Raw*.cpp
	rm -f *.o
	$(CC) $(CFLAGS) $(INCLUDE) -Iinclude/mobj/*.h -I$(ID_MOBJ_DOMAINS)/*.h \
	-I$(ID_MOBJ_DOMAINS)/*Raw*/*.h -c $(SD_MOBJ_DOMAINS)/*Raw*.cpp
	ar rvs $(OD_MOBJ_DOMAINS)/rawfile.a *.o
	mkdir -p $(OD_MOBJ_DOMAINS)
	mv *.o $(OD_MOBJ_DOMAINS)

# Compiles the concurrent planning test program #
conc: $(ALL_CPP) $(ALL_H)
	$(CC) $(CFLAGS) $(INCLUDE) -c $(DOM_CPP)
	mkdir -p test
	mv *.o test/
	$(CC) $(CFLAGS) $(INCLUDE) -o testconc.out $(TD)/testConc.cpp \
	  $(TD)/*.o $(LIBS)
	rm test/*.o

# Compiles the Racetrack domain test program #
race: $(I_H) $(RACE_H) $(RACE_CPP) $(S_CPP) libmdp
	$(CC) $(CFLAGS) -I$(ID_RACE) -I$(ID) -c $(RACE_CPP)
	mkdir -p test
	mv *.o test/
	$(CC) $(CFLAGS) $(INCLUDE) -o testrace.out $(TD)/testRace.cpp \
	  $(TD)/*.o $(LIBS)
	rm test/*.o

# Compiles the Sailing domain test program #
sail: $(I_H) $(SAIL_H) $(SAIL_CPP) $(S_CPP) libmdp
	$(CC) $(CFLAGS) -I$(ID_SAIL) -I$(ID) -c $(SAIL_CPP)
	mkdir -p test
	mv *.o test/
	$(CC) $(CFLAGS) $(INCLUDE) -o testsail.out $(TD)/testSail.cpp \
	  $(TD)/*.o $(LIBS)
	rm test/*.o

# Compiles the Canadian Traveler Problem domain test program #
ctp: $(CTP_CPP) $(SOLV_CPP) $(UTIL_CPP) $(I_H) $(SOLV_H) $(CTP_H) libmdp
	$(CC) $(CFLAGS) -I$(ID_CTP) $(INCLUDE_CORE) -c $(CTP_CPP)
	mkdir -p test
	mv *.o test/
	$(CC) $(CFLAGS) -I$(ID_CTP) $(INCLUDE_CORE) -o testctp.out
	  \$(TD)/testCTP.cpp $(TD)/*.o $(LIBS)
	rm test/*.o

# Compiles the Gridworld domain test program #
gw: $(GW_CPP) $(SOLV_CPP) $(UTIL_CPP) $(I_H) $(SOLV_H) $(GW_H) libmdp
	$(CC) $(CFLAGS) -I$(ID_GW) -I$(ID) -I$(ID_SOLV) -c $(GW_CPP)
	mkdir -p test
	mv *.o test/
	$(CC) $(CFLAGS) -I$(ID_GW) $(INCLUDE_CORE) -o testgw.out \
	  $(TD)/testGridWorld.cpp $(SD_DOMAINS)/*.cpp $(TD)/*.o $(LIBS)
	rm test/*.o

# Compiles a test program for a simple binary tree domain  #
b2t: $(BT_CPP) $(SOLV_CPP) $(UTIL_CPP) $(I_H) $(SOLV_H) $(BT_H) libmdp
	$(CC) $(CFLAGS) -I$(ID_BT) $(INCLUDE_CORE) -c $(BT_CPP)
	mkdir -p test
	mv *.o test/
	$(CC) $(CFLAGS) -I$(ID_BT) $(INCLUDE_CORE) -o testb2t.out \
	  $(TD)/testB2T.cpp $(TD)/*.o $(LIBS)
	rm test/*.o

domains: lib/libmdp_domains.a
lib/libmdp_domains.a: lib/libmdp.a $(DOM_H) $(DOM_CPP)
	$(CC) $(CFLAGS) $(INCLUDE) -c $(DOM_CPP)
	mkdir -p $(OD_DOMAINS)
	mv *.o $(OD_DOMAINS)
	ar rvs lib/libmdp_domains.a $(OD_DOMAINS)/*.o

testsolver.out: lib/libmdp.a domains
	$(CC) $(CFLAGS) $(INCLUDE) -o testsolver.out $(TD)/testSolver.cpp $(LIBS)

testvpi.out: lib/libmdp.a domains
	$(CC) $(CFLAGS) $(INCLUDE) -o testvpi.out $(TD)/testVPISolver.cpp $(LIBS) \
		src/solvers/VISolver.cpp

# Compiles the mini-gpt library
minigpt: lib/libminigpt.a
lib/libminigpt.a: include/ppddl/mini-gpt/*
	$(MAKE) -C include/ppddl/mini-gpt
	ar rvs lib/libminigpt.a include/ppddl/mini-gpt/*.o

# Compiles the PPDDL library
ppddl: planserv
planserv: lib/libmdp.a src/ppddl/*.cpp include/ppddl/*.h \
lib/libminigpt.a test/testPPDDL.cpp test/testClient.cpp test/planningServer.cpp
	$(CC) $(CFLAGS) -Iinclude -Iinclude/ppddl -Include/ppddl/mini-gpt -I$(ID_SOLV) -c src/ppddl/*.cpp
	mkdir -p $(TD)
	ar rvs lib/libmdp_ppddl.a *.o
	mkdir -p $(OD_PPDDL)
	mv *.o $(OD_PPDDL)
	$(CC) $(CFLAGS) -Iinclude -I$(ID_SOLV) -I$(ID_UTIL) $(INCLUDE_PPDDL) \
	  -o testppddl.out $(TD)/testPPDDL.cpp include/ppddl/mini-gpt/heuristics.cc \
	  -Llib lib/libmdp_ppddl.a lib/libmdp.a lib/libminigpt.a
	$(CC) $(CFLAGS) -Iinclude -I$(ID_SOLV) -I$(ID_UTIL) $(INCLUDE_PPDDL) \
	  -o testclient $(TD)/testClient.cpp include/ppddl/mini-gpt/heuristics.cc \
	  -Llib lib/libmdp.a lib/libminigpt.a lib/libmdp_ppddl.a
	$(CC) $(CFLAGS) -Iinclude -I$(ID_SOLV) -I$(ID_UTIL) $(INCLUDE_PPDDL) \
	  -o planserv $(TD)/planningServer.cpp include/ppddl/mini-gpt/heuristics.cc \
	  -Llib  lib/libmdp_ppddl.a lib/libmdp.a lib/libminigpt.a

# Compiles the reduced model code
reduced: lib/libmdp_reduced.a
lib/libmdp_reduced.a: lib/libmdp.a domains ppddl $(SD_REDUCED)/*.cpp $(ID_REDUCED)/*.h
	$(CC) $(CFLAGS) -Iinclude -I$(ID_REDUCED) -c $(SD_REDUCED)/*.cpp
	mkdir -p $(TD)
	ar rvs lib/libmdp_reduced.a *.o
	mkdir -p $(OD_REDUCED)
	mv *.o $(OD_REDUCED)
	$(CC) $(CFLAGS) -I$(ID_REDUCED) $(INCLUDE_CORE) $(INCLUDE_PPDDL) \
	  -o testreduced.out $(TD)/reduced/testReduced.cpp $(OD_DOMAINS)/*.o \
      $(ID_PPDDL)/mini-gpt/heuristics.cc \
      $(SD)/Action.cpp \
      lib/libminigpt.a lib/libmdp_reduced.a lib/libmdp_ppddl.a $(LIBS)
#	$(CC) $(CFLAGS) -I$(ID_REDUCED) $(INCLUDE_CORE) $(INCLUDE_PPDDL) \
#      -o testReducedFF.out $(TD)/reduced/testReducedFF.cpp $(OD_DOMAINS)/*.o \
#      $(SD_SOLV)/LAOStarSolver.cpp \
#      $(SD)/Action.cpp \
#      $(ID_PPDDL)/mini-gpt/heuristics.cc \
#      $(LIBS) lib/libminigpt.a lib/libmdp_reduced.a lib/libmdp_ppddl.a
	$(CC) $(CFLAGS) -I$(ID_REDUCED) $(INCLUDE_CORE) $(INCLUDE_PPDDL) \
      -o planserv_red.out $(TD)/reduced/planningServer.cpp $(OD_DOMAINS)/*.o \
      $(SD_SOLV)/LAOStarSolver.cpp \
      $(SD)/Action.cpp \
      $(ID_PPDDL)/mini-gpt/heuristics.cc \
      $(LIBS) lib/libminigpt.a lib/libmdp_reduced.a lib/libmdp_ppddl.a
#	$(CC) $(CFLAGS) -I$(ID_REDUCED) $(INCLUDE_CORE) $(INCLUDE_PPDDL) \
#      -o testrff.out $(TD)/testRFF.cpp $(OD_DOMAINS)/*.o \
#      $(SD_SOLV)/LAOStarSolver.cpp \
#      $(SD)/Action.cpp \
#      $(ID_PPDDL)/mini-gpt/heuristics.cc \
#      $(LIBS) lib/libminigpt.a lib/libmdp_reduced.a lib/libmdp_ppddl.a

.PHONY: clean
clean:
	rm -f $(TD)/*.o
	rm -f *.o
	rm -f $(OD)/*.a
	rm -f $(OD)/*.o
	rm -f $(OD)/core/*.o
	rm -f $(OD)/domains/*.o
	rm -f $(OD)/domains/*.a
	rm -f $(OD)/domains/mobj/*
	rm -f $(OD)/solvers/*.o
	rm -f $(OD)/solvers/*.a
	rm -f $(OD)/solvers/mobj/*
	rm -f $(ID_PPDDL)/mini-gpt/*.o
	rm -f lib/libmdp*.a
