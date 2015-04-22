#include <iostream>
#include <ctime>
#include <fstream>
#include <sstream>
#include <unistd.h>

#include "../include/domains/racetrack/RacetrackAction.h"
#include "../include/lexi/domains/MORTrackDetHeuristic.h"
#include "../include/lexi/domains/MORacetrackProblem.h"
#include "../include/lexi/domains/MORacetrackState.h"
#include "../include/solvers/solver.h"
#include "../include/solvers/CMDPLinProgSolver.h"
#include "../include/solvers/GlobalSlackSolver.h"
#include "../include/solvers/LexiVISolver.h"
#include "../include/solvers/MOLAOStarSolver.h"
#include "../include/util/general.h"
#include "../include/util/graph.h"


using namespace mlcore;
using namespace mlsolvers;
using namespace mlmobj;
using namespace std;

int main(int argc, char* args[])
{
    if (argc < 5) {
        cerr << "Usage ./textlexirace TRACK ALGORITHM SLACK SAFETY";
        cerr << " --optional [NSIMS VERBOSITY GAMMA]" << endl;
        exit(0);
    }

    mdplib_debug = true;

    double slack = atof(args[3]);
    double gamma = argc > 7 ? atof(args[7]) : 0.95;
    bool useSafety = atoi(args[4]);
    int verbosity = argc > 6 ? atoi(args[6]) : 1;
    int size = 3;

    MOProblem* problem = new MORacetrackProblem(args[1], size);
    ((MORacetrackProblem*) problem)->setPError(0.00);
    ((MORacetrackProblem*) problem)->setPSlip(0.20);
    ((MORacetrackProblem*) problem)->setMDS(0);
    ((MORacetrackProblem*) problem)->useSafety(useSafety);
    problem->slack(slack);

    vector<Heuristic*> heuristics;
    Heuristic* heuristic =
        (strcmp(args[2], "vi") == 0) ? nullptr : new MORTrackDetHeuristic(args[1], useSafety);
    for (int i = 0; i < size; i++)
        heuristics.push_back(heuristic);
    problem->heuristics(heuristics);

    clock_t startTime = clock();
    double tol = 1.0e-6;
    if (strcmp(args[2], "global") == 0) {
        GlobalSlackSolver gss(problem, tol, 1000000);
        gss.solve(problem->initialState());
    } else if (strcmp(args[2], "lp") == 0) {
        vector<double> targets(2);
        targets[0] = 10000; targets[1] = 10000;
        CMDPLinProgSolver lp(problem, targets);
        lp.solve(problem->initialState());
    } else if (strcmp(args[2], "lao") == 0) {
        MOLAOStarSolver lao(problem, tol, 1000000);
        lao.solve(problem->initialState());
    } else if (strcmp(args[2], "vi") == 0) {
        problem->gamma(gamma);
        problem->slack(slack * (1 - gamma));
        problem->generateAll();
        if (verbosity > 1)
            cerr << problem->states().size() << " states" << endl;
        LexiVISolver vi(problem, 1000000000, tol);
        vi.solve();
    }
    clock_t endTime = clock();
    if (verbosity > 10) {
        cerr << "Estimated cost "
             << ((MOState *) problem->initialState())->mobjCost()[0] << " "
             << ((MOState *) problem->initialState())->mobjCost()[1] << endl;
        cerr << startTime << " " << endTime << endl;
        cerr << "Time " << ((endTime - startTime + 0.0) / CLOCKS_PER_SEC) << endl;
    } else if (verbosity > 1) {
        cerr << ((MOState *) problem->initialState())->mobjCost()[0] << " "
             << ((MOState *) problem->initialState())->mobjCost()[1] << endl;
    }

    int nsims = argc > 5 ? atoi(args[5]) : 1;
    vector <double> expectedCost(2, 0.0);
    mdplib_debug = false;
    for (int i = 0; i < nsims; i++) {
        State* tmp = problem->initialState();
        if (verbosity > 100) {
            cerr << " ********* Simulation Starts ********* " << endl;
        }
        while (!problem->goal(tmp)) {
            Action* a;
            a = tmp->bestAction();

            if (verbosity > 100) {
                MOState* lex = (MOState *) tmp;
                cerr << endl << "STATE-ACTION *** " << tmp << " " << a << " " << endl;
                double c0 = problem->cost(lex,a,0), c1 = problem->cost(lex,a,1);
                cerr << lex->mobjCost()[0] << " " <<  lex->mobjCost()[1];
                cerr << " - costs " << c0 << " " << c1 << endl;
            }

            expectedCost[0] += problem->cost(tmp, a, 0);
            expectedCost[1] += problem->cost(tmp, a, 1);
            tmp = randomSuccessor(problem, tmp, a);
        }
        if (verbosity > 100)
            cerr << endl;
    }

    if (verbosity > 0)
        cerr << "Avg. Exec cost " << expectedCost[0] / nsims << " " << expectedCost[1] / nsims << endl;

    delete problem;
    delete ((MORTrackDetHeuristic*) heuristic);
}

