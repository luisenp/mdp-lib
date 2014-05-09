CC = g++
CFLAGS = -std=c++11

GridWorld: src/domains/gridworld/*.cpp include/*.h include/domains/gridworld/*.h
	$(CC) $(CFLAGS) -Iinclude/domains/gridworld -Iinclude -c -g src/domains/gridworld/*.cpp

Debug: src/*.cpp
	$(CC) $(CFLAGS) -c -g src/*.cpp

