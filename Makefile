CC = g++
CFLAGS = -std=c++11 -g -DATOM_STATES -pthread

GWSDIR = src/domains/gridworld
GWIDIR = include/domains/gridworld

CTPSDIR = src/domains/ctp
CTPIDIR = include/domains/ctp

SAILSDIR = src/domains/sailing
SAILIDIR = include/domains/sailing

INCLUDE = -I$(GWIDIR) -I$(CTPIDIR) -I$(SAILIDIR) -Iinclude/domains/ -Iinclude -Iinclude/solvers -Include/util

conc: $(GWSDIR)/*.cpp src/solvers/*.cpp src/util/*.cpp include/*.h include/solvers/*.h $(GWIDIR)/*.h include/domains/*.h $(CTPIDIR)/*.h
	$(CC) $(CFLAGS) $(INCLUDE) -c $(GWSDIR)/*.cpp $(CTPSDIR)/*.cpp src/domains/*.cpp src/util/*.cpp src/*.cpp src/solvers/*.cpp
	mv *.o test/
	$(CC) $(CFLAGS) $(INCLUDE) -o testconc test/testConc.cpp test/*.o lib/libminigpt.a

sail: include/*.h $(SAILIDIR)/*.h $(SAILSDIR)/*.cpp  src/*.cpp
	$(CC) $(CFLAGS) -I$(SAILIDIR) -Iinclude -c $(SAILSDIR)/*.cpp
	# mv *.o test/
	# $(CC) $(CFLAGS) $(INCLUDE) -o testsail test/testsail.cpp test/*.o lib/libminigpt.a

ctp: src/domains/ctp/*.cpp src/solvers/*.cpp src/util/*.cpp include/*.h include/solvers/*.h include/domains/ctp/*.h
	$(CC) $(CFLAGS) -Iinclude/domains/ctp -Iinclude -Iinclude/solvers -Include/util -c src/domains/ctp/*.cpp src/util/*.cpp src/*.cpp src/solvers/*.cpp
	mv *.o test/
	$(CC) $(CFLAGS) -Iinclude/domains/ctp -Iinclude -Iinclude/solvers -Iinclude/util -o testctp test/testCTP.cpp test/*.o lib/libminigpt.a

gw: $(GWSDIR)/*.cpp src/solvers/*.cpp src/util/*.cpp include/*.h include/solvers/*.h $(GWIDIR)/*.h
	$(CC) $(CFLAGS) -Iinclude/domains/gridworld -Iinclude -Iinclude/solvers -c $(GWSDIR)/*.cpp src/util/*.cpp src/*.cpp src/solvers/*.cpp
	mv *.o test/
	$(CC) $(CFLAGS) -Iinclude/domains/gridworld -Iinclude -Iinclude/solvers -Iinclude/util -o testgw test/testGridWorld.cpp test/*.o

b2t: src/domains/binarytree/*.cpp src/solvers/*.cpp src/util/*.cpp include/*.h include/solvers/*.h include/domains/binarytree/*.h
	$(CC) $(CFLAGS) -Iinclude/domains/binarytree -Iinclude -Iinclude/solvers -Include/util -c src/domains/binarytree/*.cpp src/util/*.cpp src/*.cpp src/solvers/*.cpp
	mv *.o test/
	$(CC) $(CFLAGS) -Iinclude/domains/binarytree -Iinclude -Iinclude/solvers -Iinclude/util -o testb2t test/testB2T.cpp test/*.o

ofiles: src/solvers/*.cpp src/util/*.cpp include/*.h include/solvers/*.h src/*.cpp
	$(CC) $(CFLAGS) -fPIC -shared -Iinclude -Iinclude/solvers -Include/util src/util/*.cpp src/*.cpp src/solvers/*.cpp -o libmdp.so

ppddl: src/ppddl/*.cpp include/*.h include/ppddl/*.h include/ppddl/mini-gpt/*.h src/solvers/*.cpp src/util/*.cpp
	$(CC) $(CFLAGS) -Iinclude -Iinclude/ppddl -Include/ppddl/mini-gpt -Iinclude/solvers -c src/ppddl/*.cpp src/*.cpp src/solvers/*.cpp src/util/*.cpp
	mv *.o test/
	$(CC) $(CFLAGS) -Iinclude -Iinclude/solvers -Iinclude/util -L. -lmdp -o testppddl test/testPPDDL.cpp test/*.o lib/libminigpt.a

clean: test/*.o
	rm test/*.o
	rm *.o

