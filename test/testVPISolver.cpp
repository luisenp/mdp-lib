#include <climits>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>

#include "../include/solvers/HMinHeuristic.h"
#include "../include/solvers/VPIRTDPSolver.h"

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
VPIRTDPSolver* solver = nullptr;
string algorithm = "vpi-rtdp";

int verbosity = 0;

void setupRacetrack()
{
    string trackName = flag_value("track");
    if (verbosity > 100)
        cout << "Setting up racetrack " << trackName << endl;
    int mds = -1;
    if (flag_is_registered_with_value("mds"))
        mds = stoi(flag_value("mds"));
    problem = new RacetrackProblem(trackName.c_str());
    ((RacetrackProblem*) problem)->pError(0.05);
    ((RacetrackProblem*) problem)->pSlip(0.10);
    ((RacetrackProblem*) problem)->mds(mds);
    if (!flag_is_registered_with_value("heuristic") ||
            flag_value("heuristic") == "domain")
        heuristic = new RTrackDetHeuristic(trackName.c_str());
}


void setupGridWorld()
{
    string grid = flag_value("grid");
    if (verbosity > 100)
        cout << "Setting up grid world " << grid << endl;
    bool all_directions = flag_is_registered("gw-all-dir");
    problem = new GridWorldProblem(grid.c_str(), 1.0, 50.0, all_directions);
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

void initSolver()
{
    assert(flag_is_registered_with_value("algorithm"));
    assert(flag_is_registered_with_value("trials"));
    algorithm = flag_value("algorithm");

    if (flag_is_registered("dead-end-cost")) {
        mdplib::dead_end_cost = stof(flag_value("dead-end-cost"));
    }

    double tol = 1.0e-3;
    int trials = 1000;
    if (flag_is_registered_with_value("trials"))
        trials = stoi(flag_value("trials"));
    if (flag_is_registered_with_value("tol"))
        tol = stof(flag_value("tol"));

    if (algorithm == "rtdp") {
        // RTDP with upper bound action selection
        // is just VPI-RTDP with vanillaSample set to true
        solver = new VPIRTDPSolver(problem, tol, trials,
                                   0.0, 0.0, 0.0,
                                   mdplib::dead_end_cost,
                                   true);
    } else if (algorithm == "brtdp") {
        // BRTDP is just VPI-RTDP with beta = 0
        double tau = 100;
        solver = new VPIRTDPSolver(problem, tol, trials,
                                   -1.0, 0.0, tau,
                                   mdplib::dead_end_cost);
    } else if (algorithm == "vpi-rtdp") {
        double alpha = 1.0;
        double beta = 0.95 * mdplib::dead_end_cost;
        double tau = 100;
        if (flag_is_registered_with_value("beta"))
            beta = stof(flag_value("beta"));
        if (flag_is_registered_with_value("alpha"))
            alpha = stof(flag_value("alpha"));
        solver = new VPIRTDPSolver(problem,
                                   tol, trials,
                                   alpha, beta, tau,
                                   mdplib::dead_end_cost);
        if (flag_is_registered("vpi-delta"))
            solver->sampleVPIDelta();
        else if (flag_is_registered("vpi-old"))
            solver->sampleVPIOld();
    } else {
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
    if (flag_is_registered_with_value("dead-end-cost"))
        mdplib::dead_end_cost = stof(flag_value("dead-end-cost"));
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

    int numSims = 100;    // How many times a policy is run
    if (flag_is_registered_with_value("n"))
        numSims = stoi(flag_value("n"));
    // How many time an experiment is repeated. A full experiment consists
    // on the planner performing |numTrials|, and the resulting policy being
    // run |numSims| times.
    int numReps = 10;
    if (flag_is_registered_with_value("reps"))
        numReps = stoi(flag_value("reps"));

    // Running simulations to evaluate the solver's performance.
    StateSet statesSeen;

    int maxTrials = stoi(flag_value("trials"));
    for (int numTrials = maxTrials; numTrials <= maxTrials; numTrials += 10) {
        double expectedCostTrials = 0.0;
        double varianceTrials = 0.0;
        double averageTimePlanning = 0.0;
        double varianceTimePlanning = 0.0;
        for (int rep = 0; rep < numReps; rep++) {
            // Each repetition is a full experiment.
            // Planner performs |numTrials| and then the policy is run
            // |numSims| times.
            solver->reset();
            solver->maxTrials(numTrials);
            double expectedCostPolicy = 0.0;
            double variancePolicy = 0.0;
            double timePlanning = 0.0;
            int cntSims = 0;
            for (int i = 0; i < numSims; i++) {
                if (verbosity >= 100)
                    cout << " ********* Simulation Starts ********* " << endl;
                clock_t startTime, endTime;
                if (i == 0) {
                    for (State* s : problem->states())
                        s->reset();
                    startTime = clock();
                    solver->solve(problem->initialState());
                    endTime = clock();
                    timePlanning =
                        (double(endTime - startTime) / CLOCKS_PER_SEC);
                }
                if (verbosity >= 10) {
                    cout << "Starting simulation " << i << endl;
                }
                State* tmp = problem->initialState();
                if (verbosity >= 100) {
                    cout << "Estimated cost " <<
                        problem->initialState()->cost() << endl << tmp << " ";
                }
                double costPolicy = 0.0;
                int plausTrial = 0;
                while (!problem->goal(tmp)) {
                    statesSeen.insert(tmp);
                    Action* a = tmp->bestAction();
                    if (verbosity >= 1000) {
                        cout << "State/Action: "
                            << tmp << " " << a << " " << endl;
                    }
                    costPolicy += problem->cost(tmp, a);
                    costPolicy = std::min(costPolicy, mdplib::dead_end_cost);
                    if (costPolicy >= mdplib::dead_end_cost) {
                        break;
                    }
                    double prob = 0.0;
                    State* aux = randomSuccessor(problem, tmp, a, &prob);
                    tmp = aux;
                }
                if (flag_is_registered("ctp")) {
                    CTPState* ctps = static_cast<CTPState*>(tmp);
                    if (!ctps->badWeather()) {
                        cntSims++;
                        updateStatistics(costPolicy, cntSims,
                                         expectedCostPolicy, variancePolicy);
                    }
                } else {
                    cntSims++;
                    updateStatistics(costPolicy, cntSims,
                                     expectedCostPolicy, variancePolicy);
                }
                if (verbosity >= 100)
                    cout << endl;
            }
            updateStatistics(expectedCostPolicy, rep + 1,
                             expectedCostTrials, varianceTrials);
            updateStatistics(timePlanning, rep + 1,
                             averageTimePlanning, varianceTimePlanning);
        }
        if (verbosity >= 1) {
            cout << "Expected cost for " << numTrials << " trials: "
                << expectedCostTrials
                << " Avg. time planning: " << averageTimePlanning << endl;
        } else if (verbosity == 0) {
            cout << numTrials << " " << expectedCostTrials
                << " " << sqrt(varianceTrials)
                << " " << averageTimePlanning << endl;
        }
    }

    delete problem;
    delete heuristic;
    delete solver;
}


