CC = g++
CFLAGS = -std=c++11 -g -DATOM_STATES

ctp: src/domains/ctp/*.cpp src/solvers/*.cpp src/util/*.cpp include/*.h include/solvers/*.h include/domains/ctp/*.h
	$(CC) $(CFLAGS) -Iinclude/domains/ctp -Iinclude -Iinclude/solvers -Include/util -c src/domains/ctp/*.cpp src/util/*.cpp src/*.cpp src/solvers/*.cpp
	mv *.o test/
	$(CC) $(CFLAGS) -Iinclude/domains/ctp -Iinclude -Iinclude/solvers -Iinclude/util -o testctp test/testCTP.cpp test/*.o

gw: src/domains/gridworld/*.cpp src/solvers/*.cpp src/util/*.cpp include/*.h include/solvers/*.h include/domains/gridworld/*.h
	$(CC) $(CFLAGS) -Iinclude/domains/gridworld -Iinclude -Iinclude/solvers -c src/domains/gridworld/*.cpp src/util/*.cpp src/*.cpp src/solvers/*.cpp
	mv *.o test/
	$(CC) $(CFLAGS) -Iinclude/domains/gridworld -Iinclude -Iinclude/solvers -Iinclude/util -o testgw test/testGridWorld.cpp test/*.o

b2t: src/domains/binarytree/*.cpp src/solvers/*.cpp src/util/*.cpp include/*.h include/solvers/*.h include/domains/binarytree/*.h
	$(CC) $(CFLAGS) -Iinclude/domains/binarytree -Iinclude -Iinclude/solvers -Include/util -c src/domains/binarytree/*.cpp src/util/*.cpp src/*.cpp src/solvers/*.cpp
	mv *.o test/
	$(CC) $(CFLAGS) -Iinclude/domains/binarytree -Iinclude -Iinclude/solvers -Iinclude/util -o testb2t test/testB2T.cpp test/*.o

ofiles: src/solvers/*.cpp src/util/*.cpp include/*.h include/solvers/*.h src/*.cpp
	$(CC) $(CFLAGS) -fPIC -shared -Iinclude -Iinclude/solvers -Include/util src/util/*.cpp src/*.cpp src/solvers/*.cpp -o libmdp.so

ppddl: src/ppddl/*.cpp include/*.h include/ppddl/*.h include/ppddl/mini-gpt/*.h
	$(CC) $(CFLAGS) -Iinclude -Iinclude/ppddl -Include/ppddl/mini-gpt -c src/ppddl/*.cpp

clean: test/*.o
	rm test/*.o
	rm *.o

