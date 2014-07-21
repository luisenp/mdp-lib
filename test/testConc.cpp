#include <chrono>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <cstdio>

#include "../include/util/general.h"
#include "../include/domains/gridworld/GridWorldState.h"
#include "../include/domains/gridworld/GridWorldProblem.h"
#include "../include/domains/gridworld/GridWorldAction.h"
#include "../include/domains/gridworld/GWManhattanHeuristic.h"

#include "../include/solvers/ConcurrentSolver.h"
#include "../include/solvers/LRTDPSolver.h"
#include "../include/solvers/VISolver.h"

using namespace std;
using namespace mlsolvers;
using namespace mlcore;

int main(int argc, char *args[])
{
    int n = atoi(args[1]);
    PairDoubleMap goals;
    goals.insert(make_pair(pair<int,int> (n - 1,n - 1), -1.0));
    Problem* problem = new GridWorldProblem(n, n, 0, 0, &goals);
    GridWorldState* s0 = (GridWorldState *) problem->initialState();
    Heuristic* heuristic = new GWManhattanHeuristic((GridWorldProblem*) problem);

    LRTDPSolver lrtdp(problem, 1, 1.0e-1);
    VISolver vi(problem, 1, 1.0e-1);


//    for (int i = 0; i < 1000; i++) {
//        lrtdp.solve(problem->initialState());
//    }
//    State* cur = problem->initialState();
//    while (true) {
//        Action* a = cur->bestAction();
//        if (a == nullptr) {
//            cerr << "Not enough planning time" << endl;
//            return 0;
//        }
//
//        cerr << cur << " " << a << endl;
//        cur = randomSuccessor(problem, cur, a);
//        if (problem->goal(cur)) {
//            cerr << "GOAL!!" << endl;
//            return 0;
//        }
//    }

    ConcurrentSolver* solver = new ConcurrentSolver(lrtdp);
    solver->setState(problem->initialState());
    mutex& solverMutex = solver->solverThreadMutex();

    solver->run();
    this_thread::sleep_for(chrono::milliseconds( 1000 ));

    State* cur = problem->initialState();
    double cost = 0;
    while (true) {
        solverMutex.lock();

        Action* a = cur->bestAction();

        if (a == nullptr) {
            solverMutex.unlock();
            this_thread::sleep_for(chrono::milliseconds( 50 ));
            continue;
        }

        cost += problem->cost(cur, a);
        cur = randomSuccessor(problem, cur, a);
        solver->setState(cur);

        cerr << cur << " " << a << " " << cost << endl;

        if (problem->goal(cur)) {
            cerr << "Finished with cost " << cost << endl;

            solver->setKeepRunning(false);
            solverMutex.unlock();
            delete solver;

            return 0;
        }

        solverMutex.unlock();
        this_thread::sleep_for(chrono::milliseconds( 500));
    }
}
