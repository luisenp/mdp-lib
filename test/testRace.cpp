#include <iostream>
#include <ctime>
#include <fstream>
#include <sstream>
#include <unistd.h>

#include "../include/solvers/DeterministicSolver.h"
#include "../include/solvers/EpicSolver.h"
#include "../include/solvers/LAOStarSolver.h"
#include "../include/solvers/LRTDPSolver.h"
#include "../include/solvers/Solver.h"
#include "../include/solvers/UCTSolver.h"
#include "../include/solvers/VISolver.h"

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

//    cerr << problem->states().size() << " states" << endl;

    Heuristic* heuristic = new RTrackDetHeuristic(trackName.c_str());

    problem->setHeuristic(heuristic);

    Solver* solver;
    clock_t startTime = clock();
    double tol = 1.0e-6;
    assert(flag_is_registered_with_value("algorithm"));
    string algorithm = flag_value("algorithm");
    if (algorithm == "wlao") {
        double weight = 1.0;
        if (flag_is_registered_with_value("weight"))
            weight = stof(flag_value("weight"));
        solver = new LAOStarSolver(problem, tol, 1000000, weight);
    } else if (algorithm == "lao") {
        solver = new LAOStarSolver(problem, tol, 1000000);
    } else if (algorithm == "lrtdp") {
        solver = new LRTDPSolver(problem, 1000000000, tol);
    } else if (algorithm == "vi") {
        solver = new VISolver(problem, 1000000000, tol);
    } else if (algorithm == "epic") {
        int horizon = 0, expansions = 1;
        if (flag_is_registered_with_value("horizon"))
            horizon = stoi(flag_value("horizon"));
        if (flag_is_registered_with_value("expansions"))
            horizon = stoi(flag_value("expansions"));
        solver = new EpicSolver(problem, horizon, expansions);
    } else if (algorithm == "det") {
        solver = new DeterministicSolver(problem,
                                         mlsolvers::det_most_likely,
                                         heuristic);
    } else {
        cerr << "Unknown algorithm: " << algorithm << endl;
        return -1;
    }
    solver->solve(problem->initialState());
    clock_t endTime = clock();

    double actionsPerSecond = 4.0;
    double totalTime = (double(endTime - startTime) / CLOCKS_PER_SEC);

//    cerr << "Estimated cost " << problem->initialState()->cost() << endl;
//    cerr << "Planning Time: " <<  totalTime << endl;

    if (algorithm == "vi") {
        for (State* s : problem->states()) {
            if (s->cost() < heuristic->cost(s)) {
                cerr << "Error: " << s << " " << s->cost() <<
                    " " << heuristic->cost(s) << endl;
            }
        }
    }

    int nsims = 10000;
    if (flag_is_registered_with_value("n"))
        nsims = stoi(flag_value("n"));
    int verbosity = 0;
    if (flag_is_registered_with_value("v"))
        verbosity = stoi(flag_value("v"));

    double expectedCost = 0.0;
    double expectedTime = 0.0;
    StateSet statesSeen;
    for (int i = 0; i < nsims; i++) {
        if (verbosity >= 10) {
            cerr << "Starting simulation " << i << endl;
        }
        State* tmp = problem->initialState();
        if (verbosity >= 100) {
            cerr << " ********* Simulation Starts ********* " << endl;
            cerr << tmp << " ";
        }
        while (!problem->goal(tmp)) {
            statesSeen.insert(tmp);
//            Action* a = tmp->bestAction();
            Action* a = greedyAction(problem, tmp);
//            if (a == nullptr) {
//                startTime = clock();
//                a = solver->solve(tmp);
//                endTime = clock();
//                expectedTime += (double(endTime - startTime) / CLOCKS_PER_SEC);
//            }
            expectedCost += problem->cost(tmp, a);
            tmp = randomSuccessor(problem, tmp, a);
            if (verbosity > 100) {
                cerr << a << " " << endl;
                cerr << tmp << " ";
            }
        }
        if (verbosity > 100)
            cerr << endl;
    }
    expectedCost /= nsims;
    expectedTime /= nsims;

    cerr << "Avg. Exec cost " << expectedCost << " ";
    cerr << "Total time " << totalTime + expectedTime << " ";
    double expectedCostTime = actionsPerSecond * (totalTime + expectedTime);
    cerr << "Avg. Total cost " << expectedCost + expectedCostTime << " ";
    cerr << "States seen " << statesSeen.size() << endl;

    delete problem;
    delete ((RTrackDetHeuristic*) heuristic);
}
