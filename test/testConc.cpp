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
#include "../include/domains/WrapperProblem.h"
#include "../include/domains/DummyState.h"

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

    WrapperProblem* wrapper = new WrapperProblem(problem);
    DummyState* dummy = wrapper->dummyState();

    LRTDPSolver lrtdp(wrapper, 1, 1.0e-1);
    VISolver vi(wrapper, 1, 1.0e-1);

    ConcurrentSolver* solver = new ConcurrentSolver(lrtdp);
    solver->setState(wrapper->initialState());
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
        dummy->setSuccessors(problem->transition(cur, a));
        solver->setState(dummy);

        cerr << cur << " " << a << " " << cost << endl;

        if (problem->goal(cur)) {
            cerr << "Finished with cost " << cost << endl;

            solver->setKeepRunning(false);
            solverMutex.unlock();
            delete solver;
            delete wrapper;

            return 0;
        }

        solverMutex.unlock();
        this_thread::sleep_for(chrono::milliseconds( 500));
    }
}
