CC = g++
CFLAGS = -std=c++11

GridWorld: src/domains/gridworld/*.cpp src/util/*.cpp include/*.h include/domains/gridworld/*.h
	$(CC) $(CFLAGS) -Iinclude/domains/gridworld -Iinclude -c -g src/domains/gridworld/*.cpp src/util/*.cpp src/*.cpp
	mv *.o test/
	$(CC) $(CFLAGS) -Iinclude/domains/gridworld -Iinclude -Iinclude/util -g -o testgw test/testGridWorld.cpp test/*.o


Debug: src/*.cpp
	$(CC) $(CFLAGS) -c -g src/*.cpp

