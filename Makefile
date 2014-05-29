CC = g++
CFLAGS = -std=c++11 -g

ctp: src/domains/ctp/*.cpp src/solvers/*.cpp src/util/*.cpp include/*.h include/solvers/*.h include/domains/ctp/*.h
	$(CC) $(CFLAGS) -Iinclude/domains/ctp -Iinclude -Iinclude/solvers -Include/util -c src/domains/ctp/*.cpp src/util/*.cpp src/*.cpp src/solvers/*.cpp
	mv *.o test/
	$(CC) $(CFLAGS) -Iinclude/domains/ctp -Iinclude -Iinclude/solvers -Iinclude/util -o testctp test/testCTP.cpp test/*.o

gw: src/domains/gridworld/*.cpp src/solvers/*.cpp src/util/*.cpp include/*.h include/solvers/*.h include/domains/gridworld/*.h
	$(CC) $(CFLAGS) -Iinclude/domains/gridworld -Iinclude -Iinclude/solvers -c src/domains/gridworld/*.cpp src/util/*.cpp src/*.cpp src/solvers/*.cpp
	mv *.o test/
	$(CC) $(CFLAGS) -Iinclude/domains/gridworld -Iinclude -Iinclude/solvers -Iinclude/util -o testgw test/testGridWorld.cpp test/*.o

clean: test/*.o
	rm test/*.o

