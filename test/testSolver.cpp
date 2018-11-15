#include <climits>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <unistd.h>

#include "../include/solvers/AODetHeuristic.h"
#include "../include/solvers/BoundedRTDPSolver.h"
#include "../include/solvers/DeterministicSolver.h"
#include "../include/solvers/HDPSolver.h"
#include "../include/solvers/HMinHeuristic.h"
#include "../include/solvers/HOPSolver.h"
#include "../include/solvers/LAOStarSolver.h"
#include "../include/solvers/LRTDPSolver.h"
#include "../include/solvers/FLARESSolver.h"
#include "../include/solvers/SoftFLARESSolver.h"
#include "../include/solvers/Solver.h"
#include "../include/solvers/SSiPPSolver.h"
#include "../include/solvers/UCTSolver.h"
#include "../include/solvers/VISolver.h"
#include "../include/solvers/VPIRTDPSolver.h"

#include "../include/util/flags.h"
#include "../include/util/general.h"
#include "../include/util/graph.h"

#include "../include/domains/borderexit/BorderExitProblem.h"

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
bool useUpperBound = false;

int verbosity = 0;
bool useOnline = false;

///////////////////////////////////////////////////////////////////////////////
//                              PROBLEM SETUP                                //
///////////////////////////////////////////////////////////////////////////////
void setupRacetrack()
{
    string trackName = flag_value("track");
    if (verbosity > 100)
        cout << "Setting up racetrack " << trackName << endl;
    int mds = -1;
    if (flag_is_registered_with_value("mds"))
        mds = stoi(flag_value("mds"));
    double perror = 0.05;
    if (flag_is_registered_with_value("perror"))
        perror = stod(flag_value("perror"));
    double pslip = 0.10;
    if (flag_is_registered_with_value("pslip"))
        pslip = stod(flag_value("pslip"));
    problem = new RacetrackProblem(trackName.c_str());
    ((RacetrackProblem*) problem)->pError(perror);
    ((RacetrackProblem*) problem)->pSlip(pslip);
    ((RacetrackProblem*) problem)->mds(mds);
    if (!flag_is_registered_with_value("heuristic") ||
            flag_value("heuristic") == "domain")
        heuristic = new RTrackDetHeuristic(trackName.c_str());
}

void setupBorderExitProblem()
{
    problem = new BorderExitProblem();
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

    // The old version was all outcomes probability 0.20
    static double windTransition[] = {
        0.30, 0.20, 0.15, 0.00, 0.00, 0.00, 0.15, 0.20,
        0.20, 0.30, 0.20, 0.15, 0.00, 0.00, 0.00, 0.15,
        0.15, 0.20, 0.30, 0.20, 0.15, 0.00, 0.00, 0.00,
        0.00, 0.15, 0.20, 0.30, 0.20, 0.15, 0.00, 0.00,
        0.00, 0.00, 0.15, 0.20, 0.30, 0.20, 0.15, 0.00,
        0.00, 0.00, 0.00, 0.15, 0.20, 0.30, 0.20, 0.15,
        0.15, 0.00, 0.00, 0.00, 0.15, 0.20, 0.30, 0.20,
        0.20, 0.15, 0.00, 0.00, 0.00, 0.15, 0.20, 0.30};

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
    } else if (flag_is_registered("border-problem")) {
        setupBorderExitProblem();
    } else {
        cerr << "Invalid problem." << endl;
        exit(-1);
    }
}


///////////////////////////////////////////////////////////////////////////////
//                                  SOLVER SETUP                             //
///////////////////////////////////////////////////////////////////////////////

// Initializes the given solver according to the given algorithm.
void initSolver(string algorithm, Solver*& solver)
{
    double tol = 1.0e-3;
    assert(flag_is_registered_with_value("algorithm"));

    if (flag_is_registered("dead-end-cost")) {
        mdplib::dead_end_cost = stof(flag_value("dead-end-cost"));
    }

    int horizon = 0, expansions = 1, trials = 1000000;
    if (flag_is_registered_with_value("horizon"))
        horizon = stoi(flag_value("horizon"));
    if (flag_is_registered_with_value("expansions"))
        expansions = stoi(flag_value("expansions"));
    if (flag_is_registered_with_value("trials"))
        trials = stoi(flag_value("trials"));
    if (flag_is_registered_with_value("tol"))
        tol = stof(flag_value("tol"));

    if (algorithm == "wlao") {
        double weight = 1.0;
        if (flag_is_registered_with_value("weight"))
            weight = stof(flag_value("weight"));
        solver = new LAOStarSolver(problem, tol, 1000000, weight);
    } else if (algorithm == "lao") {
        solver = new LAOStarSolver(problem, tol, 1000000);
    } else if (algorithm == "lrtdp") {
        solver = new LRTDPSolver(problem, trials, tol, -1);
    } else if (algorithm == "brtdp") {
        double ub = 0.0;
        if (flag_is_registered_with_value("ub"))
            ub = stof(flag_value("ub"));
        solver = new BoundedRTDPSolver(problem, 0.01, ub, 10, trials);
        useUpperBound = true;
    } else if (algorithm == "rtdp-ub") {
        // RTDP with upper bound action selection
        // is just VPI-RTDP with vanillaSample set to true
        solver = new VPIRTDPSolver(problem, tol, trials,
                                   0.0, 0.0, 0.0,
                                   mdplib::dead_end_cost + 10.0,
                                   true);
        useUpperBound = true;
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
            static_cast<VPIRTDPSolver*>(solver)->sampleVPIDelta();
        else if (flag_is_registered("vpi-old"))
            static_cast<VPIRTDPSolver*>(solver)->sampleVPIOld();
        useUpperBound = true;
    } else if (algorithm == "flares") {
        bool optimal = flag_is_registered("optimal");
        bool useProbsDepth = flag_is_registered("use-prob-depth");
        double depth = horizon;
        if (flag_is_registered("prob"))
            depth = stof(flag_value("prob"));
        solver = new FLARESSolver(
            problem, trials, tol, depth, optimal, useProbsDepth);
    } else if (algorithm == "soft-flares") {
        double depth = horizon;
        double alpha = 0.10;
        bool optimal = flag_is_registered("optimal");
        TransitionModifierFunction mod_func = kLogistic;
        DistanceFunction dist_func = kStepDist;
        HorizonFunction horizon_func = kFixed;
        if (flag_is_registered_with_value("alpha"))
            alpha = stof(flag_value("alpha"));
        // Distance functions
        if (flag_is_registered("dist")) {
            string dist_str = flag_value("dist");
            if (dist_str == "traj") {
                dist_func = kTrajProb;
            } else if (dist_str == "plaus") {
                dist_func = kPlaus;
            } else if (dist_str == "depth") {
                dist_func = kStepDist;
            } else {
                cerr << "Error: unknown distance function." << endl;
                exit(0);
            }
        }
        // Labeling functions
        if (flag_is_registered("labelf")) {
            string labelf_str = flag_value("labelf");
            if (labelf_str == "exp") {
                mod_func = kExponential;
            } else if (labelf_str == "step") {
                mod_func = kStep;
            } else if (labelf_str == "linear") {
                mod_func = kLinear;
            } else if (labelf_str == "logistic") {
                mod_func = kLogistic;
            } else {
                cerr << "Error: unknown labeling function." << endl;
                exit(0);
            }
        }
        // Horizon functions (to allow some probability of deeper exploration)
        double psi = 0.2;
        if (flag_is_registered("horf")) {
            string horf_str = flag_value("horf");
            if (horf_str == "exp") {
                horizon_func = kExponentialH;
            } else if (horf_str == "fixed") {
                horizon_func = kFixed;
            } else if (horf_str == "bern") {
                horizon_func = kBernoulli;
            } else {
                cerr << "Error: unknown labeling function." << endl;
                exit(0);
            }
        } else if (optimal) {
            if (flag_is_registered_with_value("psi"))
                psi = stof(flag_value("psi"));
            horizon_func = kBernoulli;
        }
        solver = new SoftFLARESSolver(
            problem, trials, tol, depth, mod_func, dist_func, horizon_func,
            alpha, false, false, optimal, psi);
    } else if (algorithm == "rtdp") {
        solver = new SoftFLARESSolver(
            problem, trials, tol, 0, kLinear, kStepDist, kFixed,
            0.0, false, true);
    } else if (algorithm == "hdp") {
        int plaus;
        if (flag_is_registered_with_value("i"))
            solver = new HDPSolver(problem, tol, stoi(flag_value("i")));
        else
            solver = new HDPSolver(problem, tol);
    } else if (algorithm == "vi") {
        solver = new VISolver(problem, 1000000000, tol);
    } else if (algorithm == "ssipp") {
        double rho = -1.0;
        bool useTrajProb = false;
        if (flag_is_registered_with_value("rho")) {
            rho = stof(flag_value("rho"));
            useTrajProb = true;
        }
        solver = new SSiPPSolver(problem, tol, horizon, SSiPPAlgo::Original);
        SSiPPSolver* ssipp = static_cast<SSiPPSolver*> (solver);
        ssipp->maxTrials(1);
        ssipp->useTrajProbabilities(useTrajProb);
        ssipp->rho(rho);
    } else if (algorithm == "labeled-ssipp") {
        double rho = -1.0;
        bool useTrajProb = false;
        if (flag_is_registered_with_value("rho")) {
            rho = stof(flag_value("rho"));
            useTrajProb = true;
        }
        solver = new SSiPPSolver(problem, tol, horizon, SSiPPAlgo::Labeled);
        SSiPPSolver* ssipp = static_cast<SSiPPSolver*> (solver);
        ssipp->useTrajProbabilities(useTrajProb);
        ssipp->rho(rho);
    } else if (algorithm == "det") {
        solver = new DeterministicSolver(problem,
                                         mlsolvers::det_most_likely,
                                         heuristic);
    } else if (algorithm == "hop") {
        solver = new HOPSolver(problem);
        if (!flag_is_registered("heuristic")
                || flag_value("heuristic") != "aodet") {
            cerr << "HOPSolver only works with --heuristic=aodet" << endl;
            exit(0);
        }
    } else if (algorithm == "uct") {
        int rollouts = 1000;
        int cutoff = 50;
        int delta = 5;
        double C = 0.0;
        bool use_qvalues_for_c = true;
        if (flag_is_registered_with_value("rollouts"))
            rollouts = stoi(flag_value("rollouts"));
        if (flag_is_registered_with_value("cutoff"))
            cutoff = stoi(flag_value("cutoff"));
        if (flag_is_registered_with_value("delta"))
            delta = stoi(flag_value("delta"));
        if (flag_is_registered("cexp")) {
            C = stod(flag_value("cexp"));
            use_qvalues_for_c = false;
        }
        solver = new UCTSolver(problem,
                               rollouts, cutoff, C,
                               use_qvalues_for_c, delta,
                               true);
    } else if (algorithm != "greedy") {
        cerr << "Unknown algorithm: " << algorithm << endl;
        exit(-1);
    }
}


///////////////////////////////////////////////////////////////////////////////
//                              SIMULATION CODE                              //
///////////////////////////////////////////////////////////////////////////////
void updateStatistics(double cost, int n, double& mean, double& M2)
{
    double delta = cost - mean;
    mean += delta / n;
    M2 += delta * (cost - mean);
}

bool mustReplan(Solver* solver, string algorithm, State* s, int plausTrial) {
    if (algorithm == "flares") {
        return !s->checkBits(mdplib::SOLVED_FLARES);
    }
    if (algorithm == "lrtdp") {
        return !s->checkBits(mdplib::SOLVED);
    }
    if (algorithm == "soft-flares") {
        if (s->checkBits(mdplib::SOLVED))
            return false;
        SoftFLARESSolver* flares = static_cast<SoftFLARESSolver*>(solver);
        return !flares->labeledSolved(s);
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
    if (algorithm == "ssipp" || algorithm == "labeled-ssipp") {
        return !s->checkBits(mdplib::SOLVED_SSiPP);
    }
    if (algorithm == "uct" || algorithm == "rtdp" || algorithm == "hop") {
        return true;
    }
    if (flag_is_registered("online"))
        return true;
    if (useUpperBound)
        return s->bestAction() == nullptr;
    return false;
}

bool mustResetPlanner(int i) {
    if (flag_is_registered("reset-every-trial"))
        return true;
    if (flag_is_registered("no-initial-plan"))
        return false;
    // Otherwise only compute an initial plan during the first simulation
    // and re-plan if necessary in the subsequent simulations.
    return (i == 0);
}

// Runs [numSims] of the given solver and and returns the results
// (i.e., expectedCost, variance, totalTime, statesSeen).
// Argument [algorithm] is the name of the algorithm implemented by [solver].
// Argument [maxTime], if set to > 0, specifies the maximum time allowed to
// the algorithm to complete all simulations (in milliseconds).
// If [perReplan] is passed, then [maxTime] is used as the maximum time allowed
// per re-planning event.
vector<double> simulate(Solver* solver,
                        string algorithm,
                        int numSims,
                        int maxTime = -1,
                        bool perReplan = false)
{
    double expectedCost = 0.0;
    double variance = 0.0;
    double totalTime = 0.0;
    double longestTime = 0.0;
    double expectedTime = 0.0;  // expected *total* time
    double varianceTime = 0.0;  // variance *total* time
    StateSet statesSeen;
    int cnt = 0;
    int numDecisions = 0;
    clock_t simulationsStartTime = clock();
    for (int i = 0; i < numSims; i++) {
        if (verbosity >= 100)
            cout << " ********* Simulation Starts ********* " << endl;
        clock_t startTime, endTime;
        double simulationPlanTime = 0.0;
        // If requested, reset all state information computed by the algorithm
        if (mustResetPlanner(i)) {
            for (State* s : problem->states())
                s->reset();
            if (maxTime > 0) {
                solver->maxPlanningTime(maxTime);
            }
            solver->reset();
            if (!flag_is_registered("precompute-h"))
                heuristic->reset();
            startTime = clock();
            // Initial planning
            if (algorithm != "greedy")
                solver->solve(problem->initialState());

            endTime = clock();
            double planTime = (double(endTime - startTime) / CLOCKS_PER_SEC);
            totalTime += planTime;
            simulationPlanTime += planTime;
            longestTime = std::max(longestTime, planTime);
            numDecisions++;
        }
        if (verbosity >= 10) {
            cout << "Starting simulation " << i << endl;
        }
        State* tmp = problem->initialState();
        if (verbosity >= 100) {
            cout << "Estimated cost " <<
                problem->initialState()->cost() << endl;
        }
        // This is where the actual simulated trial starts
        double costTrial = 0.0;
        int plausTrial = 0;
        while (!problem->goal(tmp)) {
            statesSeen.insert(tmp);
            Action* a;
            // Re-planning
            if (mustReplan(solver, algorithm, tmp, plausTrial)) {
                startTime = clock();
                int simulationsElapsedTime =
                    std::ceil(1000 * (double(startTime - simulationsStartTime)
                                / CLOCKS_PER_SEC));
                if (maxTime > -1) {
                    int planningTime = perReplan ?
                        maxTime : std::max(0, maxTime - simulationsElapsedTime);
                    solver->maxPlanningTime(planningTime);
                }
                if (algorithm != "greedy")
                    a = solver->solve(tmp);
                                                                                dprint("found action" , (void *) a);
                endTime = clock();
                double planTime =
                    (double(endTime - startTime) / CLOCKS_PER_SEC);
                totalTime += planTime;
                simulationPlanTime += planTime;
                longestTime = std::max(longestTime, planTime);
                numDecisions++;
                if (algorithm != "hop")
                    a = greedyAction(problem, tmp);
            } else {
                if (useUpperBound) {
                    // The algorithms that use upper bounds store the
                    // greedy action with respect to the upper bound
                    // in State::bestAction_
                    a = tmp->bestAction();
                }
                else {
                    a = greedyAction(problem, tmp);
                }
            }

            if (verbosity >= 1000) {
                cout << "State/Action: " << tmp << " " << a << " " << endl;
            }

            costTrial += problem->cost(tmp, a);
            costTrial = std::min(costTrial, mdplib::dead_end_cost);
            if (costTrial >= mdplib::dead_end_cost) {
                break;
            }
            double prob = 0.0;
            State* aux = randomSuccessor(problem, tmp, a, &prob);
            if (algorithm == "hdp") {
                double maxProb = 0.0;
                for (auto const & sccr : problem->transition(tmp, a))
                    maxProb = std::max(maxProb, sccr.su_prob);
                plausTrial +=
                    static_cast<HDPSolver*>(solver)->kappa(prob, maxProb);
            }
            tmp = aux;
        }
        if (verbosity >= 10)
            cout << costTrial << endl;
        if (flag_is_registered("ctp")) {
            CTPState* ctps = static_cast<CTPState*>(tmp);
            if (!ctps->badWeather()) {
                cnt++;
                updateStatistics(costTrial, cnt, expectedCost, variance);
                updateStatistics(
                    simulationPlanTime, cnt, expectedTime, varianceTime);
            }
        } else {
            cnt++;
            updateStatistics(costTrial, cnt, expectedCost, variance);
            updateStatistics(
                simulationPlanTime, cnt, expectedTime, varianceTime);
        }
        if (verbosity >=0) {
            if (cnt % 500 == 0 || i == numSims - 1) {
                double reportedTime = perReplan ?
                    totalTime / numDecisions : totalTime;
                cout << "sim " << cnt << " exp.cost " << expectedCost
                     << " var " << variance / (cnt - 1)
                     << " time " << reportedTime
                     << " longestTime  " << longestTime << " "
                     << " Exp[total time] " << expectedTime
                     << " Var[total time] " << varianceTime / (cnt - 1) << endl;
            }
        }
    }

    double reportedTime = perReplan ? totalTime / numDecisions : totalTime;

    if (verbosity >= 10) {
        cout << "Estimated cost " << problem->initialState()->cost() << " ";
        cout << "Avg. Exec cost " << expectedCost << " ";
        cout << "Std. Dev. " << sqrt(variance / (cnt - 1)) << " ";
        cout << "Total time " << totalTime / cnt << " " << endl;
        cout << "States seen " << statesSeen.size() << endl;
        cout << "Avg. time per decision "
             << totalTime / numDecisions << endl
             << "Longest planning time " << longestTime << endl;
        cout << "Num. decisions " << numDecisions << endl;
    }

    double results[] = {expectedCost,
                        variance / (cnt - 1),
                        reportedTime,
                        longestTime};
    return vector<double>(results, results + sizeof(results) / sizeof(double));
}


///////////////////////////////////////////////////////////////////////////////
//                                     MAIN                                  //
///////////////////////////////////////////////////////////////////////////////
void printStats(int t,
        double avgCost, double avgVar, double avgTime, double avgLongestTime,
        double M2Cost, double M2Time, double M2LongestTime, int numReps) {
    cout << t << " "
        << avgCost << " "
        << avgVar << " "
        << sqrt(M2Cost / (numReps * (numReps - 1))) << " "
        << avgTime << " "
        << sqrt(M2Time / (numReps * (numReps - 1))) << " "
        << avgLongestTime << " "
        << sqrt(M2LongestTime / (numReps * (numReps - 1))) << endl;
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
            bool solveAll = flag_is_registered("precompute-h");
            heuristic = new HMinHeuristic(problem, solveAll);
            clock_t endTime = clock();
            if (verbosity > 100) {
                cout << "Heuristic took " <<
                    (double(endTime - startTime) / CLOCKS_PER_SEC) <<
                    " seconds."  << endl;
                cout << "Cost of initial state "
                    << problem->initialState()->cost() << endl;
            }
        } else if (flag_value("heuristic") == "aodet") {
            bool precompute = flag_is_registered("precompute-h");
            heuristic = new AODetHeuristic(problem, precompute);
        } else if (flag_value("heuristic") == "zero")
            heuristic = nullptr;
    }
    problem->setHeuristic(heuristic);

    if (verbosity > 100)
        cout << problem->states().size() << " states" << endl;

    int numSims = 100;
    if (flag_is_registered_with_value("n"))
        numSims = stoi(flag_value("n"));
    int numReps = 1;
    if (flag_is_registered_with_value("reps"))
        numReps = stoi(flag_value("reps"));

    // Running simulations to evaluate each algorithm's performance
    string algorithm = flag_value("algorithm");
    stringstream ss(algorithm);
    string alg_item;
    while (getline(ss, alg_item, ',')) {
        // cout << setw(10) << alg_item << ": ";
        Solver* solver = nullptr;
        initSolver(alg_item, solver);
        // Maximum planning time per simulation in milliseconds
        int maxTime = -1;
        if (flag_is_registered_with_value("max_time")) {
            maxTime = stoi(flag_value("max_time"));
        }
        int minTime = maxTime;
        if (maxTime != -1 && flag_is_registered_with_value("min_time")) {
            minTime = stoi(flag_value("min_time"));
        }
        bool perReplan = flag_is_registered("per_replan");
        for (int t = minTime; t <= maxTime; t *= 2) {
            double avgCost = 0.0, avgVar = 0.0;
            double avgTime = 0.0, avgLongestTime = 0.0;
            double M2Cost = 0.0, M2Time = 0.0, M2LongestTime = 0.0;
            double M2Var = 0.0;
            for (int i = 1; i <= numReps; i++) {
                std::vector<double> results =
                    simulate(solver, alg_item, numSims, t, perReplan);
                updateStatistics(results[0], i, avgCost, M2Cost);
                updateStatistics(results[1], i, avgVar, M2Var);
                updateStatistics(results[2], i, avgTime, M2Time);
                updateStatistics(results[3], i, avgLongestTime, M2LongestTime);
            }
            // Prints <max_plan_time, avg_exp_cost, stderr_exp_cost,
            //           avg_var_cost,
            //           avg_exp_time, stderr_exp_time,
            //           avg_max_time, stderr_max_time>
            // One experiment is [numSims] simulations of the algorithm, which
            // outputs <exp_cost, var_cost, exp_time, max_time>.
            // The averages below are over [numReps] repetitions of an
            // experiment.
            printStats(t, avgCost, avgVar, avgTime, avgLongestTime,
                       M2Cost, M2Time, M2LongestTime, numReps);
            if (maxTime == -1)
                break;
        }
        delete solver;
    }

    delete problem;
    delete heuristic;
}

