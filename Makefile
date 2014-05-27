CC = g++
CFLAGS = -std=c++11 -g

GridWorld: src/domains/gridworld/*.cpp src/solvers/*.cpp src/util/*.cpp include/*.h include/solvers/*.h include/domains/gridworld/*.h
	$(CC) $(CFLAGS) -Iinclude/domains/gridworld -Iinclude -Iinclude/solvers -c src/domains/gridworld/*.cpp src/util/*.cpp src/*.cpp src/solvers/*.cpp
	mv *.o test/
	$(CC) $(CFLAGS) -Iinclude/domains/gridworld -Iinclude -Iinclude/solvers -Iinclude/util -o testgw test/testGridWorld.cpp test/*.o


Debug: src/*.cpp
	$(CC) $(CFLAGS) -c src/*.cpp

