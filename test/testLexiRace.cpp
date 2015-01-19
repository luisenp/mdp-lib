#include <iostream>
#include <ctime>
#include <fstream>
#include <sstream>
#include <unistd.h>

#include "../include/solvers/solver.h"
#include "../include/solvers/LexiVISolver.h"
#include "../include/solvers/LexiLAOStarSolver.h"

#include "../include/util/general.h"
#include "../include/util/graph.h"

#include "../include/lexi/domains/LexiRacetrackProblem.h"
#include "../include/lexi/domains/LexiRacetrackState.h"
#include "../include/lexi/domains/LexiRTrackDetHeuristic.h"
#include "../include/domains/racetrack/RacetrackAction.h"

using namespace mlcore;
using namespace mlsolvers;
using namespace mllexi;
using namespace std;

int main(int argc, char* args[])
{
    if (argc <= 3) {
        cerr << "Usage ./textlexirace TRACK ALGORITHM SLACK --optional [NSIMS VERBOSITY]" << endl;
        exit(0);
    }

    mdplib_debug = false;

    double slack = atof(args[3]);

    LexiProblem* problem = new LexiRacetrackProblem(args[1], 2);
    ((LexiRacetrackProblem*) problem)->setPError(0.10);
    ((LexiRacetrackProblem*) problem)->setPSlip(0.20);
    ((LexiRacetrackProblem*) problem)->setMDS(-1);
    problem->slack(slack);
    problem->generateAll();

    vector<Heuristic*> heuristics;
    Heuristic* heuristic = new LexiRTrackDetHeuristic(args[1]);
    heuristics.push_back(heuristic);
    heuristics.push_back(heuristic);
    problem->heuristics(heuristics);

    cerr << problem->states().size() << " states" << endl;

    clock_t startTime = clock();
    double tol = 1.0e-6;
    if (strcmp(args[2], "lao") == 0) {
        LexiLAOStarSolver lao(problem, tol, 1000000);
        lao.solve(problem->initialState());
    } else if (strcmp(args[2], "vi") == 0) {
        dprint1("SOLVE V!!");
        LexiVISolver vi(problem, 1000000000, tol);
        vi.solve();
        dprint1("SOLVED IT YEAH!!");
    }
    cerr << "Estimated cost "
         << ((LexiState *) problem->initialState())->lexiCost()[0] << " "
         << ((LexiState *) problem->initialState())->lexiCost()[1] << endl;
    clock_t endTime = clock();
    cerr << "Time " << ((endTime - startTime + 0.0) / CLOCKS_PER_SEC) << endl;

    int nsims = argc > 4 ? atoi(args[4]) : 1;
    int verbosity = argc > 5 ? atoi(args[5]) : 1;
    vector <double> expectedCost(2, 0.0);
    for (int i = 0; i < nsims; i++) {
        State* tmp = problem->initialState();
        if (verbosity > 100) {
            cerr << " ********* Simulation Starts ********* " << endl;
        }
        while (!problem->goal(tmp)) {
            Action* a;
            a = tmp->bestAction();

            if (verbosity > 100) {
                cerr << endl << "STATE-ACTION *** " << tmp << " " << a << " " << endl;
                lexiBellmanUpdate(problem, (LexiState *) tmp);
                double copt = ((LexiState *) tmp)->lexiCost()[0];
                double qval = qvalue(problem, (LexiState*) tmp, a, 0);
                cerr << copt << " " << qval << " " << (qval / copt - 1.0) << endl;
            }

            expectedCost[0] += problem->cost(tmp, a, 0);
            expectedCost[1] += problem->cost(tmp, a, 1);
            tmp = randomSuccessor(problem, tmp, a);
        }
        if (verbosity > 100)
            cerr << endl;
    }

    cerr << "Avg. Exec cost " << expectedCost[0] / nsims << " " << expectedCost[1] / nsims << endl;

    delete problem;
    delete ((LexiRTrackDetHeuristic*) heuristic);
}

