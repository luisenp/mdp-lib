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

#include "../include/solvers/DeterministicSolver.h"
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
    int initialPlanningT = 250;
    int noopPlanningT = 0;
    int actionT = 250;
    double kappa = actionT;
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

    double windTransition2[] = {
        0.20, 0.20, 0.20, 0.00, 0.00, 0.00, 0.20, 0.20,
        0.20, 0.20, 0.20, 0.20, 0.00, 0.00, 0.00, 0.20,
        0.20, 0.20, 0.20, 0.20, 0.20, 0.00, 0.00, 0.00,
        0.00, 0.20, 0.20, 0.20, 0.20, 0.20, 0.00, 0.00,
        0.00, 0.00, 0.20, 0.20, 0.20, 0.20, 0.20, 0.00,
        0.00, 0.00, 0.00, 0.20, 0.20, 0.20, 0.20, 0.20,
        0.20, 0.00, 0.00, 0.00, 0.20, 0.20, 0.20, 0.20,
        0.20, 0.20, 0.00, 0.00, 0.00, 0.20, 0.20, 0.20};

    /* ************************************************************************************** */
    /*                                  Setting up the problem                                */
    /* ************************************************************************************** */
    if (argc < 3) {
        cerr << "Usage: testconc [algorithm] [domain-name] [problem-instance] [parameters]" << endl;
        return 1;
    }

    if (strcmp(args[2], "grid") == 0) {
        int n = atoi(args[3]);
        goals.insert(make_pair(pair<int,int> (n - 1,n - 1), -1.0));
        problem = new GridWorldProblem(n, n, 0, 0, &goals, 0.03);
    } else if (strcmp(args[2], "ctp") == 0) {
        problem = new CTPProblem(args[3]);
        heuristic = new CTPOptimisticHeuristic((CTPProblem *) problem);
        problem->setHeuristic(heuristic);
    } else if (strcmp(args[2], "sail") == 0) {
        costs.push_back(1);
        costs.push_back(2);
        costs.push_back(4);
        costs.push_back(8);
        costs.push_back(mdplib::dead_end_cost + 1);
        int size = atoi(args[3]);
        int goal = atoi(args[4]);
        problem = new SailingProblem(0, 0, 1, goal, goal, size, size, costs, windTransition2);
        problem->generateAll();
        heuristic = new SailingNoWindHeuristic((SailingProblem *) problem);
        problem->setHeuristic(heuristic);
    } else if (strcmp(args[2], "race") == 0) {
        problem = new RacetrackProblem(args[3]);
        ((RacetrackProblem*) problem)->setPError(0.10);
        ((RacetrackProblem*) problem)->setPSlip(0.20);
        ((RacetrackProblem*) problem)->setMDS(-1);
        problem->generateAll();
        heuristic = new RTrackDetHeuristic(args[3]);
        problem->setHeuristic(heuristic);
    } else {
        cerr << "Usage: testconc [algorithm] [domain-name] [problem-instance] [parameters]" << endl;
        return 1;
    }

    /* ************************************************************************************** */
    /*                            Setting up concurrent planner                               */
    /* ************************************************************************************** */
    WrapperProblem* wrapper = new WrapperProblem(problem);
    DummyState* dummy = wrapper->dummyState();

    ConcurrentSolver* solver;
    LRTDPSolver lrtdp(wrapper, 1, 1.0e-3);
    LAOStarSolver lao(wrapper, 1.0e-3, -1);
    LAOStarSolver wlao(wrapper, 1.0e-3, -1, 5.0);
    DeterministicSolver det(problem, mlsolvers::det_most_likely, heuristic);
    if (strcmp(args[1], "lrtdp") == 0) {
        solver = new ConcurrentSolver(lrtdp);
    } else if (strcmp(args[1], "lao") == 0) {
        solver = new ConcurrentSolver(lao);
    } else if (strcmp(args[1], "wlao") == 0) {
        solver = new ConcurrentSolver(wlao);
    } else if (strcmp(args[1], "det") == 0) {
        solver = new ConcurrentSolver(det); // not really used, only here to avoid null pointers
        solver->setKeepRunning(false);
    }

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
    double costPlan = (double) initialPlanningT / kappa;
    double costExec = 0.0;
    while (true) {
        solverMutex.lock();

        if (problem->goal(cur)) {
            if (verbosity > 0)
                cerr << costExec << " " << costPlan << endl;

            solver->setKeepRunning(false);
            solverMutex.unlock();

            delete solver;
            delete problem;
            if (heuristic != nullptr) {
                if (strcmp(args[2], "ctp") == 0)
                    delete (CTPOptimisticHeuristic*) heuristic;
                else if (strcmp(args[2], "sail") == 0)
                    delete (SailingNoWindHeuristic*) heuristic;
                else if (strcmp(args[2], "race") == 0)
                    delete (RTrackDetHeuristic*) heuristic;
            }

            return 0;
        }

        /* Choosing action an updating cost */
        Action* a;
        double costCurAction;
        if (strcmp(args[1], "det") == 0) {
            clock_t time1 = clock();
            a = det.solve(cur);
            clock_t time2 = clock();
            costExec += problem->cost(cur, a);  // action cost
            costPlan += (double(time2 - time1) / CLOCKS_PER_SEC) * 1000 / kappa;
        } else {
            a = cur->bestAction();
            costCurAction = problem->cost(cur, a);
            costExec += costCurAction;
        }

        if (a == nullptr) {
            solverMutex.unlock();
            if (verbosity > 100)
                cerr << "No Action! " << cur << endl;

            a = greedyAction(problem, cur);

            assert(a != nullptr);
        }

        if (verbosity > 100)
            cerr << cur << " --- " << a << endl;

        dummy->setSuccessors(problem->transition(cur, a));
        wrapper->setDummyAction(a);
        solver->setState(dummy);

        cur = randomSuccessor(problem, cur, a);

        if (verbosity > 100)
            cerr << "    succ: " << cur << " " << costExec << " " << costPlan << endl;

        solverMutex.unlock();

        if (strcmp(args[1], "det") != 0)
            this_thread::sleep_for(chrono::milliseconds( int(costCurAction) * actionT ));

        if (verbosity > 100)
            cerr << "Executing Action " << endl;
    }
}
