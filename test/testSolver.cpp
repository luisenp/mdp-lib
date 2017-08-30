#include <climits>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>

#include "../include/solvers/DeterministicSolver.h"
#include "../include/solvers/HDPSolver.h"
#include "../include/solvers/HMinHeuristic.h"
#include "../include/solvers/LAOStarSolver.h"
#include "../include/solvers/LRTDPSolver.h"
#include "../include/solvers/FLARESSolver.h"
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


Problem* problem = nullptr;
Heuristic* heuristic = nullptr;
Solver* solver = nullptr;
string algorithm = "greedy";

int verbosity = 0;
bool useOnline = false;


void setupRacetrack()
{
    string trackName = flag_value("track");
    if (verbosity > 100)
        cout << "Setting up racetrack " << trackName << endl;
    problem = new RacetrackProblem(trackName.c_str());
    ((RacetrackProblem*) problem)->pError(0.20);
    ((RacetrackProblem*) problem)->pSlip(0.10);
    ((RacetrackProblem*) problem)->mds(-1);
    if (!flag_is_registered_with_value("heuristic") ||
            flag_value("heuristic") == "domain")
        heuristic = new RTrackDetHeuristic(trackName.c_str());
}


void setupGridWorld()
{
    string grid = flag_value("grid");
    if (verbosity > 100)
        cout << "Setting up grid world " << grid << endl;
    problem = new GridWorldProblem(grid.c_str(), 1.0, 50.0, true);
    if (!flag_is_registered_with_value("heuristic") ||
            flag_value("heuristic") == "domain")
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

    if (!flag_is_registered_with_value("heuristic") ||
            flag_value("heuristic") == "domain")
        heuristic =
            new SailingNoWindHeuristic(static_cast<SailingProblem*>(problem));
}


void setupCTP()
{
    if (verbosity > 100) {
        cout << "Setting up Canadian Traveler Problem " <<
            flag_value("ctp") << endl;
    }
    problem = new CTPProblem(flag_value("ctp").c_str());
    if (!flag_is_registered_with_value("heuristic") ||
            flag_value("heuristic") == "domain")
        heuristic =
            new CTPOptimisticHeuristic(static_cast<CTPProblem*> (problem));
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


bool mustReplan(State* s, int plausTrial) {
  if (flag_is_registered("online"))
      return true;
  if (algorithm == "flares") {
      return !s->checkBits(mdplib::SOLVED_FLARES);
  }
  if (algorithm == "hdp") {
      if (flag_is_registered("i")) {
          int j = INT_MAX;
          if (flag_is_registered_with_value("j")) {
              j = stoi(flag_value("j"));
          }
          if (plausTrial >= j) {
              static_cast<HDPSolver*>(solver)->clearLabels();
              return true;
          }
      }
  }
  if (algorithm == "ssipp") {
    return true;
  }
  return false;
}


void initSolver()
{
    double tol = 1.0e-3;
    assert(flag_is_registered_with_value("algorithm"));
    algorithm = flag_value("algorithm");

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
    } else if (algorithm == "flares") {
        bool optimal = flag_is_registered("optimal");
        bool useProbsDepth = flag_is_registered("use-prob-depth");
        double depth = horizon;
        if (flag_is_registered("prob")) {
            depth = stof(flag_value("prob"));
        }
        solver = new FLARESSolver(problem,
                                  trials,
                                  tol,
                                  depth,
                                  optimal,
                                  useProbsDepth);
    } else if (algorithm == "hdp") {
        int plaus;
        if (flag_is_registered_with_value("i"))
            solver = new HDPSolver(problem, tol, stoi(flag_value("i")));
        else
            solver = new HDPSolver(problem, tol);
    } else if (algorithm == "vi") {
        solver = new VISolver(problem, 1000000000, tol);
    } else if (algorithm == "ssipp") {
        solver = new SSiPPSolver(problem, tol, horizon);
    } else if (algorithm == "labeled-ssipp") {
        solver = new SSiPPSolver(problem, tol, horizon, SSiPPAlgo::Labeled);
    } else if (algorithm == "det") {
        solver = new DeterministicSolver(problem,
                                         mlsolvers::det_most_likely,
                                         heuristic);
    } else if (algorithm == "uct") {
        int rollouts = 1000;
        int cutoff = 50;
        int delta = 5;
        if (flag_is_registered_with_value("rollouts"))
            rollouts = stoi(flag_value("rollouts"));
        if (flag_is_registered_with_value("cutoff"))
            cutoff = stoi(flag_value("cutoff"));
        if (flag_is_registered_with_value("delta"))
            delta = stoi(flag_value("delta"));
        solver = new UCTSolver(problem, rollouts, cutoff, 0.0, true, delta);
    } else if (algorithm != "greedy") {
        cerr << "Unknown algorithm: " << algorithm << endl;
        exit(-1);
    }
}


void updateStatistics(double cost, int n, double& mean, double& M2)
{
    double delta = cost - mean;
    mean += delta / n;
    M2 += delta * (cost - mean);
}


int main(int argc, char* args[])
{
    register_flags(argc, args);

    verbosity = 0;
    if (flag_is_registered_with_value("v"))
        verbosity = stoi(flag_value("v"));
    if (flag_is_registered("debug"))
        mdplib_debug = true;
    setupProblem();
    if (!flag_is_registered("dont-generate"))
        problem->generateAll();
    if (flag_is_registered_with_value("heuristic")) {
        if (flag_value("heuristic") == "hmin") {
            clock_t startTime = clock();
            bool solveAll = flag_is_registered("hmin-solve-all");
            heuristic = new HMinHeuristic(problem, solveAll);
            clock_t endTime = clock();
            if (verbosity > 100) {
                cout << "Heuristic took " <<
                    (double(endTime - startTime) / CLOCKS_PER_SEC) <<
                    " seconds."  << endl;
            }
        } else if (flag_value("heuristic") == "zero")
            heuristic = nullptr;
    }
    problem->setHeuristic(heuristic);

    if (verbosity > 100)
        cout << problem->states().size() << " states" << endl;

    initSolver();

    int nsims = 100;
    if (flag_is_registered_with_value("n"))
        nsims = stoi(flag_value("n"));

    // Running simulations to evaluate the solver's performance.
    double expectedCost = 0.0;
    double variance = 0.0;
    double expectedTime = 0.0;
    StateSet statesSeen;

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
            if (algorithm != "greedy")
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
        int plausTrial = 0;
        while (!problem->goal(tmp)) {
            statesSeen.insert(tmp);
            Action* a;
            if (mustReplan(tmp, plausTrial)) {
                startTime = clock();
                if (algorithm != "greedy")
                    a = solver->solve(tmp);
                else
                    a = greedyAction(problem, tmp);
                endTime = clock();
                expectedTime += (double(endTime - startTime) / CLOCKS_PER_SEC);
                numDecisions++;
            }
            a = greedyAction(problem, tmp);

            if (verbosity >= 1000) {
                cout << tmp << " " << a << " " << endl;
            }

            costTrial += problem->cost(tmp, a);
            if (costTrial >= mdplib::dead_end_cost) {
                break;
            }
            double prob = 0.0;
            State* aux = randomSuccessor(problem, tmp, a, &prob);
            if (flag_value("algorithm") == "hdp") {
                double maxProb = 0.0;
                for (auto const & sccr : problem->transition(tmp, a))
                    maxProb = std::max(maxProb, sccr.su_prob);
                plausTrial +=
                    static_cast<HDPSolver*>(solver)->kappa(prob, maxProb);
            }
            tmp = aux;

        }
        if (flag_is_registered("ctp")) {
            CTPState* ctps = static_cast<CTPState*>(tmp);
            if (!ctps->badWeather()) {
                cnt++;
                updateStatistics(costTrial, cnt, expectedCost, variance);
            }
        } else {
            cnt++;
            updateStatistics(costTrial, cnt, expectedCost, variance);
            if (verbosity >= 1)
                cout << costTrial << endl;
        }
        if (verbosity >= 100)
            cout << endl;
    }

    if (verbosity >= 1) {
        cout << "Estimated cost " << problem->initialState()->cost() << " ";
        cout << "Avg. Exec cost " << expectedCost << " ";
        cout << "Std. Dev. " << sqrt(variance / (cnt - 1)) << " ";
        cout << "Total time " << expectedTime / cnt << " " << endl;
        cout << "States seen " << statesSeen.size() << endl;
        cout << "Avg. time per decision " <<
            expectedTime / numDecisions << endl;
    } else {
        cout << problem->initialState()->cost() << " ";
        cout << expectedCost << " " << sqrt(variance / (cnt - 1)) << " " <<
            expectedTime / cnt << " " << expectedTime / numDecisions << endl;
    }

    delete problem;
    delete heuristic;
    delete solver;
}

