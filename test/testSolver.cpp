#include <iostream>
#include <ctime>
#include <fstream>
#include <sstream>
#include <unistd.h>

#include "../include/solvers/DeterministicSolver.h"
#include "../include/solvers/EpicSolver.h"
#include "../include/solvers/LAOStarSolver.h"
#include "../include/solvers/LRTDPSolver.h"
#include "../include/solvers/MLRTDPSolver.h"
#include "../include/solvers/Solver.h"
#include "../include/solvers/SSiPPSolver.h"
#include "../include/solvers/UCTSolver.h"
#include "../include/solvers/VISolver.h"

#include "../include/util/flags.h"
#include "../include/util/general.h"
#include "../include/util/graph.h"

#include "../include/domains/ctp/CTPOptimisticHeuristic.h"
#include "../include/domains/ctp/CTPProblem.h"
#include "../include/domains/ctp/CTPState.h"

#include "../include/domains/gridworld/GridWorldProblem.h"
#include "../include/domains/gridworld/GWManhattanHeuristic.h"

#include "../include/domains/racetrack/RacetrackProblem.h"
#include "../include/domains/racetrack/RTrackDetHeuristic.h"

#include "../include/domains/sailing/SailingNoWindHeuristic.h"
#include "../include/domains/sailing/SailingProblem.h"


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
    if (verbosity > 100)
        cout << "Setting up racetrack " << trackName << endl;
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


void setupSailingDomain()
{
    static vector<double> costs;
    costs.push_back(1);
    costs.push_back(2);
    costs.push_back(5);
    costs.push_back(10);
    costs.push_back(mdplib::dead_end_cost + 1);

    static double windTransition[] = {
        0.20, 0.20, 0.20, 0.00, 0.00, 0.00, 0.20, 0.20,
        0.20, 0.20, 0.20, 0.20, 0.00, 0.00, 0.00, 0.20,
        0.20, 0.20, 0.20, 0.20, 0.20, 0.00, 0.00, 0.00,
        0.00, 0.20, 0.20, 0.20, 0.20, 0.20, 0.00, 0.00,
        0.00, 0.00, 0.20, 0.20, 0.20, 0.20, 0.20, 0.00,
        0.00, 0.00, 0.00, 0.20, 0.20, 0.20, 0.20, 0.20,
        0.20, 0.00, 0.00, 0.00, 0.20, 0.20, 0.20, 0.20,
        0.20, 0.20, 0.00, 0.00, 0.00, 0.20, 0.20, 0.20};

    if (!flag_is_registered_with_value("sailing-goal")) {
        cerr << "Must specify sailing-goal argument flag" << endl;
        exit(-1);
    }

    int sizeSailing = atoi(flag_value("sailing-size").c_str());
    int goalSailing = atoi(flag_value("sailing-goal").c_str());

    if (verbosity > 100)
        cout << "Setting up sailing domain with size " << sizeSailing <<
            " with goal " << goalSailing << endl;

    problem =
        new SailingProblem(0, 0, 0,
                           goalSailing, goalSailing,
                           sizeSailing, sizeSailing,
                           costs,
                           windTransition);
    heuristic =
        new SailingNoWindHeuristic(static_cast<SailingProblem*>(problem));
    problem->setHeuristic(heuristic);
}


void setupCTP()
{
    if (verbosity > 100) {
        cout << "Setting up Canadian Traveler Problem " <<
            flag_value("ctp") << endl;
    }
    problem = new CTPProblem(flag_value("ctp").c_str());
    heuristic =
        new CTPOptimisticHeuristic(static_cast<CTPProblem*> (problem));
    problem->setHeuristic(heuristic);
}


void setupProblem()
{
    if (verbosity > 100)
        cout << "Setting up problem" << endl;
    if (flag_is_registered_with_value("track")) {
        setupRacetrack();
    } else if (flag_is_registered_with_value("grid")) {
        setupGridWorld();
    } else if (flag_is_registered_with_value("sailing-size")) {
        setupSailingDomain();
    } else if (flag_is_registered_with_value("ctp")) {
        setupCTP();
    } else {
        cerr << "Invalid problem." << endl;
        exit(-1);
    }
}


bool mustReplan(State* s) {
  if (flag_is_registered("online"))
      return true;
  string algorithm = flag_value("algorithm");
  if (algorithm == "epic") {
      return !(static_cast<EpicSolver*>(solver)->canReachGoal(s));
  }
  if (algorithm == "mlrtdp") {
      return !(s->checkBits(mdplib::SOLVED_MLRTDP));
  }
  return false;
}

void initSolver()
{
    double tol = 1.0e-3;
    assert(flag_is_registered_with_value("algorithm"));
    string algorithm = flag_value("algorithm");

    int horizon = 0, expansions = 1, trials = 1000;
    if (flag_is_registered_with_value("horizon"))
        horizon = stoi(flag_value("horizon"));
    if (flag_is_registered_with_value("expansions"))
        expansions = stoi(flag_value("expansions"));
    if (flag_is_registered_with_value("trials"))
        trials = stoi(flag_value("trials"));

    if (algorithm == "wlao") {
        double weight = 1.0;
        if (flag_is_registered_with_value("weight"))
            weight = stof(flag_value("weight"));
        solver = new LAOStarSolver(problem, tol, 1000000, weight);
    } else if (algorithm == "lao") {
        solver = new LAOStarSolver(problem, tol, 1000000);
    } else if (algorithm == "lrtdp") {
        solver = new LRTDPSolver(problem, trials, tol);
    }  else if (algorithm == "mlrtdp") {
        solver = new MLRTDPSolver(problem, trials, tol, horizon);
    } else if (algorithm == "vi") {
        solver = new VISolver(problem, 1000000000, tol);
    } else if (algorithm == "epic") {
        solver = new EpicSolver(problem, horizon, expansions, trials);
    } else if (algorithm == "ssipp") {
        solver = new SSiPPSolver(problem, tol, horizon);
    } else if (algorithm == "labeled-ssipp") {
        solver = new SSiPPSolver(problem, tol, horizon, SSiPPAlgo::Labeled);
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
    if (!flag_is_registered("dont-generate"))
        problem->generateAll();
    problem->setHeuristic(heuristic);

    if (verbosity > 100)
        cout << problem->states().size() << " states" << endl;

    initSolver();

    int nsims = 100;
    if (flag_is_registered_with_value("n"))
        nsims = stoi(flag_value("n"));

    // Running simulations to evaluate the solver's performance.
    double expectedCost = 0.0;
    double expectedTime = 0.0;
    StateSet statesSeen;

    if (nsims == 0) {
      clock_t startTime = clock();
      solver->solve(problem->initialState());
      clock_t endTime = clock();
      expectedTime += (double(endTime - startTime) / CLOCKS_PER_SEC);
      expectedCost += problem->initialState()->cost();
      cout << expectedCost << " " << expectedTime << " " << endl;
    }

    int cnt = 0;
    int numDecisions = 0;
    for (int i = 0; i < nsims; i++) {
        if (verbosity >= 100)
            cout << " ********* Simulation Starts ********* " << endl;
        clock_t startTime, endTime;
        if (i == 0 || !flag_is_registered("no-initial-plan")) {
            for (State* s : problem->states())
                s->reset();
            startTime = clock();
            solver->solve(problem->initialState());
            endTime = clock();
            expectedTime += (double(endTime - startTime) / CLOCKS_PER_SEC);
            numDecisions++;
        }
        if (verbosity >= 10) {
            cout << "Starting simulation " << i << endl;
        }
        State* tmp = problem->initialState();
        if (verbosity >= 100) {
            cout << "Estimated cost " <<
                problem->initialState()->cost() << endl << tmp << " ";
        }
        double costTrial = 0.0;
        while (!problem->goal(tmp)) {
            statesSeen.insert(tmp);
            Action* a;
            if (mustReplan(tmp)) {
                startTime = clock();
                a = solver->solve(tmp);
                endTime = clock();
                expectedTime += (double(endTime - startTime) / CLOCKS_PER_SEC);
                numDecisions++;
            }
            a = greedyAction(problem, tmp);
            costTrial += problem->cost(tmp, a);
            if (costTrial >= mdplib::dead_end_cost) {
                break;
            }
            tmp = randomSuccessor(problem, tmp, a);
            if (verbosity >= 1000) {
                cout << a << " " << endl;
                cout << tmp << " ";
            }
        }
        if (flag_is_registered("ctp")) {
            CTPState* ctps = static_cast<CTPState*>(tmp);
            if (!ctps->badWeather()) {
                expectedCost += costTrial;
                cnt++;
            }
        } else {
            expectedCost += costTrial;
            cnt++;
        }
        if (verbosity >= 100)
            cout << endl;
    }
    if (nsims > 0) {
        expectedCost;
        expectedTime;
    }

    if (verbosity >= 1) {
        cout << "Avg. Exec cost " << expectedCost / cnt << " ";
        cout << "Total time " << expectedTime / cnt << " " << endl;
        cout << "States seen " << statesSeen.size() << endl;
        cout << "Avg. time per decision " <<
            expectedTime / numDecisions << endl;
    } else {
        cout << expectedCost / cnt << " " << expectedTime / cnt << " " << endl;
    }

    delete problem;
    delete heuristic;
    delete solver;
}

