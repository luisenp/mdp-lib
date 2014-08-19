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

#include "../include/domains/racetrack/RacetrackState.h"
#include "../include/domains/racetrack/RacetrackProblem.h"
#include "../include/domains/racetrack/RacetrackAction.h"
#include "../include/domains/racetrack/RTrackDetHeuristic.h"

#include "../include/domains/sailing/SailingState.h"
#include "../include/domains/sailing/SailingProblem.h"
#include "../include/domains/sailing/SailingAction.h"

#include "../include/domains/WrapperProblem.h"
#include "../include/domains/DummyState.h"

#include "../include/solvers/ConcurrentSolver.h"
#include "../include/solvers/LAOStarSolver.h"
#include "../include/solvers/LRTDPSolver.h"
#include "../include/solvers/VISolver.h"

using namespace std;
using namespace mlsolvers;
using namespace mlcore;

int main(int argc, char *args[])
{
    /* Simulation parameters */
    int initialPlanningT = 10000;
    int noopPlanningT = 1000;
    int actionT = 1000;
    double kappa = 250;
    int verbosity = 1;

    Problem* problem;
    Heuristic* heuristic = nullptr;
    PairDoubleMap goals;
    vector<double> costs;

    double windTransition[] = {
        0.50, 0.15, 0.10, 0.00, 0.00, 0.00, 0.10, 0.15,
        0.15, 0.50, 0.15, 0.10, 0.00, 0.00, 0.00, 0.10,
        0.10, 0.15, 0.50, 0.15, 0.10, 0.00, 0.00, 0.00,
        0.00, 0.10, 0.15, 0.50, 0.15, 0.10, 0.00, 0.00,
        0.00, 0.00, 0.10, 0.15, 0.50, 0.15, 0.10, 0.00,
        0.00, 0.00, 0.00, 0.10, 0.15, 0.50, 0.15, 0.10,
        0.10, 0.00, 0.00, 0.00, 0.10, 0.15, 0.50, 0.15,
        0.15, 0.10, 0.00, 0.00, 0.00, 0.10, 0.15, 0.50};

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
        problem->setHeuristic(heuristic);
    } else if (strcmp(args[1], "sail") == 0) {
        costs.push_back(1);
        costs.push_back(2);
        costs.push_back(5);
        costs.push_back(10);
        costs.push_back(mdplib::dead_end_cost + 1);

        int size = atoi(args[2]);
        int goal = atoi(args[3]);
        problem = new SailingProblem(0, 0, 1, goal, goal, size, size, costs, windTransition);
        problem->generateAll();
        heuristic = new SailingNoWindHeuristic((SailingProblem *) problem);
//        problem->setHeuristic(heuristic);
    } else if (strcmp(args[1], "race") == 0) {
        problem = new RacetrackProblem(args[2]);
        problem->generateAll();
        heuristic = new RTrackDetHeuristic(args[2]);
        problem->setHeuristic(heuristic);
    } else {
        cerr << "Usage: testconc [domain-name] [problem-instance]" << endl;
        return 1;
    }

    /* ************************************************************************************** */
    /*                            Setting up concurrent planner                               */
    /* ************************************************************************************** */
    WrapperProblem* wrapper = new WrapperProblem(problem);
    DummyState* dummy = wrapper->dummyState();

    LRTDPSolver lrtdp(wrapper, 100000, 1.0e-3);
    lrtdp.setMaxChecked(10000000);
//    ConcurrentSolver* solver = new ConcurrentSolver(lrtdp);

    LAOStarSolver lao(wrapper, 1.0e-3);
    ConcurrentSolver* solver = new ConcurrentSolver(lao);

    solver->setState(wrapper->initialState());
    mutex& solverMutex = mlsolvers::bellman_mutex;

    solver->run();
    this_thread::sleep_for(chrono::milliseconds( initialPlanningT ));   // Initial planning time

    if (verbosity > 100)
        cerr << "Initial planning completed." << endl;

    /* ************************************************************************************** */
    /*                         Starting execution/planning simulation                         */
    /* ************************************************************************************** */
    State* cur = wrapper->initialState();
    double cost = (double) initialPlanningT / kappa;
    while (true) {
        solverMutex.lock();

        if (problem->goal(cur)) {
            if (verbosity > 0)
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
            if (verbosity > 100)
                cerr << "No Action! " << cur << endl;

            a = greedyAction(problem, cur);

            assert(a != nullptr);

            // At this point we know the current state, so the planner can be updated
//            list<Successor> sccrs;
//            sccrs.push_back(Successor(cur, 1.0));
//            dummy->setSuccessors(sccrs);
//            this_thread::sleep_for(chrono::milliseconds( noopPlanningT ));
//            cost += noopPlanningT / kappa;
//            continue;
        }

        if (verbosity > 100)
            cerr << cur << " --- " << a << endl;

        dummy->setSuccessors(problem->transition(cur, a));
        wrapper->setDummyAction(a);
        solver->setState(dummy);

        cost += problem->cost(cur, a);
        cur = randomSuccessor(problem, cur, a);

        if (verbosity > 100)
            cerr << "    succ: " << cur << " " << cost << endl;


        solverMutex.unlock();
        this_thread::sleep_for(chrono::milliseconds( actionT ));

        if (verbosity > 100)
            cerr << "Executing Action " << endl;
    }
}
