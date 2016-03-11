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

#include "../include/domains/gridworld/GridWorldProblem.h"
#include "../include/domains/gridworld/GWManhattanHeuristic.h"

#include "../include/domains/racetrack/RacetrackProblem.h"
#include "../include/domains/racetrack/RTrackDetHeuristic.h"


using namespace mdplib;
using namespace mlcore;
using namespace mlsolvers;
using namespace std;


Problem* problem;
Heuristic* heuristic;
Solver* solver;

int verbosity = 0;
bool useOnline = false;


void setupRacetrack()
{
    string trackName = flag_value("track");
    problem = new RacetrackProblem(trackName.c_str());
    ((RacetrackProblem*) problem)->pError(0.10);
    ((RacetrackProblem*) problem)->pSlip(0.20);
    ((RacetrackProblem*) problem)->mds(-1);
    heuristic = new RTrackDetHeuristic(trackName.c_str());
}


void setupGridWorld()
{
    string grid = flag_value("grid");
    if (verbosity > 100)
        cout << "Setting up grid world " << grid << endl;
    problem = new GridWorldProblem(grid.c_str(), 1.0);
    heuristic = new GWManhattanHeuristic((GridWorldProblem*) problem);
}


void setupProblem()
{
    if (verbosity > 100)
        cout << "Setting up problem" << endl;
    if (flag_is_registered_with_value("track")) {
        setupRacetrack();
    } else if (flag_is_registered_with_value("grid")) {
        setupGridWorld();
    } else {
        cerr << "Invalid problem." << endl;
        exit(-1);
    }
}


void initSolver()
{
    double tol = 1.0e-3;
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
        int trials = 10000000;
        if (flag_is_registered_with_value("trials"))
            trials = stoi(flag_value("trials"));
        solver = new LRTDPSolver(problem, trials, tol);
    } else if (algorithm == "vi") {
        solver = new VISolver(problem, 1000000000, tol);
    } else if (algorithm == "epic") {
        int horizon = 0, expansions = 1, trials = 1000;
        if (flag_is_registered_with_value("horizon"))
            horizon = stoi(flag_value("horizon"));
        if (flag_is_registered_with_value("expansions"))
            expansions = stoi(flag_value("expansions"));
        if (flag_is_registered_with_value("trials"))
            trials = stoi(flag_value("trials"));
        solver = new EpicSolver(problem, horizon, expansions, trials);
    } else if (algorithm == "det") {
        solver = new DeterministicSolver(problem,
                                         mlsolvers::det_most_likely,
                                         heuristic);
    } else {
        cerr << "Unknown algorithm: " << algorithm << endl;
        assert(false);
    }
}


int main(int argc, char* args[])
{
    register_flags(argc, args);

    verbosity = 0;
    if (flag_is_registered_with_value("v"))
        verbosity = stoi(flag_value("v"));

    mdplib_debug = true;
    setupProblem();
    problem->generateAll();
    problem->setHeuristic(heuristic);

    if (verbosity > 100)
        cerr << problem->states().size() << " states" << endl;

    initSolver();
//    clock_t startTime = clock();
//    solver->solve(problem->initialState());
//    clock_t endTime = clock();

//    double actionsPerSecond = 4.0;
//    double totalTime = (double(endTime - startTime) / CLOCKS_PER_SEC);

//    if (verbosity >= 1) {
//        cerr << "Estimated cost " << problem->initialState()->cost() << endl;
//        cerr << "Planning Time: " <<  totalTime << endl;
//    }

    int nsims = 100;
    if (flag_is_registered_with_value("n"))
        nsims = stoi(flag_value("n"));

    // Running simulations to evaluate the solver's performance.
    double expectedCost = 0.0;
    double expectedTime = 0.0;
    StateSet statesSeen;
    for (int i = 0; i < nsims; i++) {
        for (State* s : problem->states())
            s->reset();
        clock_t startTime = clock();
        solver->solve(problem->initialState());
        clock_t endTime = clock();
        expectedTime += (double(endTime - startTime) / CLOCKS_PER_SEC);

        if (verbosity >= 10) {
            cerr << "Starting simulation " << i << endl;
        }
        State* tmp = problem->initialState();
        if (verbosity >= 100) {
            cerr << " ********* Simulation Starts ********* " << endl;
            cerr << tmp << " ";
        }
        double costTrial = 0.0;
        while (!problem->goal(tmp)) {
            statesSeen.insert(tmp);
            Action* a;
            if (flag_is_registered("online")) {
                startTime = clock();
                a = solver->solve(tmp);
                endTime = clock();
                expectedTime += (double(endTime - startTime) / CLOCKS_PER_SEC);
            } else {
                if (flag_value("algorithm") == "epic") {
                    if (!(static_cast<EpicSolver*>(solver)->
                            canReachGoal(tmp))) {
                        startTime = clock();
                        solver->solve(tmp);
                        endTime = clock();
                        expectedTime +=
                            (double(endTime - startTime) / CLOCKS_PER_SEC);
                    }
                }
                a = greedyAction(problem, tmp);
            }
            costTrial += problem->cost(tmp, a);
            if (costTrial >= mdplib::dead_end_cost) {
                                                                                exit(-1);
                break;
            }
            tmp = randomSuccessor(problem, tmp, a);
            if (verbosity >= 1000) {
                cerr << a << " " << endl;
                cerr << tmp << " ";
            }
        }
        expectedCost += costTrial;
        if (verbosity >= 100)
            cerr << endl;
    }
    expectedCost /= nsims;
    expectedTime /= nsims;

    if (verbosity >= 1) {
        cerr << "Avg. Exec cost " << expectedCost << " ";
        cerr << "Total time " << expectedTime << " " << endl;
//        double expectedCostTime = actionsPerSecond * (totalTime + expectedTime);
//        cerr << "Avg. Total cost " << expectedCost + expectedCostTime << " ";
        cerr << "States seen " << statesSeen.size() << endl;
    } else {
        cerr << expectedCost << " " << expectedTime << " " << endl;
//        double expectedCostTime = actionsPerSecond * (totalTime + expectedTime);
//        cerr << expectedCost + expectedCostTime << endl;
    }

    delete problem;
    delete heuristic;
    delete solver;
}

