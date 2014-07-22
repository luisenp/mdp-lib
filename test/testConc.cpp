#include <chrono>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "../include/util/general.h"

#include "../include/domains/gridworld/GridWorldState.h"
#include "../include/domains/gridworld/GridWorldProblem.h"
#include "../include/domains/gridworld/GridWorldAction.h"

#include "../include/domains/ctp/CTPState.h"
#include "../include/domains/ctp/CTPProblem.h"
#include "../include/domains/ctp/CTPAction.h"
#include "../include/domains/ctp/CTPOptimisticHeuristic.h"

#include "../include/domains/WrapperProblem.h"
#include "../include/domains/DummyState.h"

#include "../include/solvers/ConcurrentSolver.h"
#include "../include/solvers/LRTDPSolver.h"
#include "../include/solvers/VISolver.h"

using namespace std;
using namespace mlsolvers;
using namespace mlcore;

int initialPlanningT = 1000;
int noopPlanningT = 500;
int actionT = 500;

int main(int argc, char *args[])
{
    Problem* problem;
    Heuristic* heuristic = nullptr;
    PairDoubleMap goals;

    /* ************************************************************************************** */
    /*                                  Setting up the problem                                */
    /* ************************************************************************************** */
    if (argc < 3) {
        cerr << "Input Error " << endl;
        return 1;
    }

    if (strcmp(args[1], "grid") == 0) {
        int n = atoi(args[2]);
        goals.insert(make_pair(pair<int,int> (n - 1,n - 1), -1.0));
        problem = new GridWorldProblem(n, n, 0, 0, &goals);
    } else if (strcmp(args[1], "ctp") == 0) {
        problem = new CTPProblem(args[2]);
        heuristic = new CTPOptimisticHeuristic((CTPProblem *) problem);
//        problem->setHeuristic(heuristic);
    } else {
        cerr << "Input Error " << args[1] << endl;
        return 1;
    }

    /* ************************************************************************************** */
    /*                            Setting up concurrent planner                               */
    /* ************************************************************************************** */
    WrapperProblem* wrapper = new WrapperProblem(problem);
    DummyState* dummy = wrapper->dummyState();

    LRTDPSolver lrtdp(wrapper, 1, 1.0e-1);

    ConcurrentSolver* solver = new ConcurrentSolver(lrtdp);
    solver->setState(wrapper->initialState());
    mutex& solverMutex = mlsolvers::bellman_mutex;

    solver->run();
    this_thread::sleep_for(chrono::milliseconds( initialPlanningT ));   // Initial planning time
    cerr << "Initial planning completed." << endl;

    /* ************************************************************************************** */
    /*                         Starting execution/planning simulation                         */
    /* ************************************************************************************** */
    State* cur = wrapper->initialState();
    double cost = 0;
    while (true) {
        solverMutex.lock();

        if (problem->goal(cur)) {
            cerr << "Finished with cost " << cost << endl;

            solver->setKeepRunning(false);
            solverMutex.unlock();
            delete solver;
            delete wrapper;
            if (heuristic != nullptr)
                delete heuristic;

            return 0;
        }

        Action* a = cur->bestAction();

        if (a == nullptr) {
            solverMutex.unlock();
            cerr << "No Action! " << cur << endl;
            this_thread::sleep_for(chrono::milliseconds( noopPlanningT ));
            continue;
        }

        cerr << cur << " --- " << a << endl;

        dummy->setSuccessors(problem->transition(cur, a));
        wrapper->setDummyAction(a);
        solver->setState(dummy);

        cost += problem->cost(cur, a);
        cur = randomSuccessor(problem, cur, a);

        cerr << "    succ: " << cur << " " << cost << endl;


        solverMutex.unlock();
        this_thread::sleep_for(chrono::milliseconds( actionT ));

        cerr << "Executing Action " << endl;
    }
}
