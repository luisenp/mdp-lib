#include <iostream>
#include <ctime>
#include <fstream>
#include <sstream>
#include <unistd.h>

#include "../include/solvers/LAOStarSolver.h"
#include "../include/solvers/Solver.h"

#include "../include/util/flags.h"
#include "../include/util/general.h"
#include "../include/util/graph.h"

#include "../include/domains/racetrack/RacetrackProblem.h"
#include "../include/domains/racetrack/RacetrackState.h"
#include "../include/domains/racetrack/RacetrackAction.h"
#include "../include/domains/racetrack/RTrackDetHeuristic.h"
#include "../include/domains/racetrack/RTrackLowResHeuristic.h"


using namespace mdplib;
using namespace mlcore;
using namespace mlsolvers;
using namespace std;


int main(int argc, char* args[])
{
    register_flags(argc, args);

    assert(flag_is_registered_with_value("track"));
    string trackName = flag_value("track");

    mdplib_debug = true;
    Problem* problem = new RacetrackProblem(trackName.c_str());
    ((RacetrackProblem*) problem)->pError(0.10);
    ((RacetrackProblem*) problem)->pSlip(0.20);
    ((RacetrackProblem*) problem)->mds(-1);
    problem->generateAll(); 

    Heuristic* heuristic = new RTrackDetHeuristic(trackName.c_str());

    problem->setHeuristic(heuristic);

    clock_t startTime = clock();
    double tol = 1.0e-6;
    Solver* solver = new LAOStarSolver(problem, tol, 1000000);
    solver->solve(problem->initialState());
    clock_t endTime = clock();

    double actionsPerSecond = 4.0;
    double totalTime = (double(endTime - startTime) / CLOCKS_PER_SEC);

    int nsims = 10000;
    double expectedCost = 0.0;
    double expectedTime = 0.0;
    StateSet statesSeen;
    for (int i = 0; i < nsims; i++) {
        State* tmp = problem->initialState();
        while (!problem->goal(tmp)) {
            statesSeen.insert(tmp);
            Action* a = greedyAction(problem, tmp);
            expectedCost += problem->cost(tmp, a);
            tmp = randomSuccessor(problem, tmp, a);
        }
    }
    expectedCost /= nsims;
    expectedTime /= nsims;

    cerr << "Avg. Exec cost " << expectedCost << " ";
    cerr << "Total time " << totalTime + expectedTime << " ";
    double expectedCostTime = actionsPerSecond * (totalTime + expectedTime);
    cerr << "States seen " << statesSeen.size() << endl;

    delete problem;
    delete ((RTrackDetHeuristic*) heuristic);
}
