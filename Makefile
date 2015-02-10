#########################################################################
#                                VARIABLES                              #
#########################################################################


# Compilation flags and variables #
CC = g++
CFLAGS = -std=c++11 -O3 -DATOM_STATES -pthread

# Variables for directories #
ID = include
SD = src
TD = test
ID_UTIL = $(ID)/util
SD_UTIL = $(SD)/util
ID_SOLV = $(ID)/solvers
SD_SOLV = $(SD)/solvers

ID_DOM = $(ID)/domains
SD_DOM = $(SD)/domains
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

# Variables for include directives #
INCLUDE_DOM = -I$(ID_GW) -I$(ID_CTP) -I$(ID_SAIL) -I$(ID_DOM) -I$(ID_RACE)
INCLUDE_CORE = -I$(ID_SOLV) -I$(ID_UTIL) -I$(ID)
INCLUDE = $(INCLUDE_DOM) $(INCLUDE_CORE)

# Variables for source/header files #
I_H = $(ID)/*.h
S_CPP = $(SD)/*.cpp
SOLV_CPP = $(SD_SOLV)/*.cpp
SOLV_H = $(ID_SOLV)/*.h
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

ALL_H = $(I_H) $(SOLV_H) $(DOM_H) $(UTIL_H)
ALL_CPP = $(DOM_CPP) $(SOLV_CPP) $(UTIL_CPP)

# Libraries
LIBS = lib/libmdp.a

#########################################################################
#                                 TARGETS                               #
#########################################################################

# Compiles the core MDP-LIB library #
libmdp: $(S_CPP) $(SOLV_CPP) $(UTIL_CPP) $(I_H) $(SOLV_H) $(UTIL_H)
	rm -f *.o
	$(CC) $(CFLAGS) $(INCLUDE_CORE) -c $(UTIL_CPP) $(S_CPP) $(SOLV_CPP) $(UTIL_CPP)
	ar rvs libmdp.a *.o
	mv libmdp.a lib
	rm *.o

# Compiles the lexicographic SSPP library #
lexi: $(ALL_CPP) $(ALL_H)
	$(CC) $(CFLAGS) $(INCLUDE) -Iinclude/lexi/*.h -Iinclude/lexi/domains/*.h -c src/lexi/domains/*.cpp $(GW_CPP)
	mv *.o test/
	$(CC) $(CFLAGS) $(INCLUDE) -o testlexirace $(TD)/testLexiRace.cpp $(TD)/*.o $(LIBS)
	$(CC) $(CFLAGS) $(INCLUDE) -o testlexigw $(TD)/testLexiGW.cpp $(TD)/*.o $(LIBS)
	$(CC) $(CFLAGS) $(INCLUDE) -o testlexiraw $(TD)/testLexiRaw.cpp $(TD)/*.o $(LIBS)
	rm test/*.o

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

ppddl: src/ppddl/*.cpp $(I_H) include/ppddl/*.h include/ppddl/mini-gpt/*.h $(SOLV_CPP) $(UTIL_CPP)
	$(CC) $(CFLAGS) -Iinclude -Iinclude/ppddl -Include/ppddl/mini-gpt -I$(ID_SOLV) -c src/ppddl/*.cpp src/*.cpp $(SOLV_CPP) $(UTIL_CPP)
	mv *.o test/
	$(CC) $(CFLAGS) -Iinclude -I$(ID_SOLV) -I$(ID_UTIL) -o testppddl test/testPPDDL.cpp test/*.o $(LIBS) lib/libminigpt.a

clean:
	rm -f test/*.o
	rm -f *.o

