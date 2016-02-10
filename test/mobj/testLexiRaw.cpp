#include <iostream>
#include <ctime>
#include <fstream>
#include <sstream>
#include <unistd.h>

#include "../include/solvers/Solver.h"
#include "../include/solvers/mobj/LexiVISolver.h"
#include "../include/solvers/mobj/MOLAOStarSolver.h"

#include "../include/util/general.h"
#include "../include/util/graph.h"

#include "../include/mobj/domains/RawFileMOProblem.h"
#include "../include/mobj/domains/RawFileMOState.h"
#include "../include/mobj/domains/RawFileMOAction.h"
#include "../include/mobj/MObjProblem.h"
#include "../include/mobj/MObjState.h"

using namespace mdplib_mobj_solvers;
using namespace mlcore;
using namespace mlsolvers;
using namespace mlmobj;
using namespace std;

int main(int argc, char* args[])
{
    if (argc <= 3) {
        cerr << "Usage ./textlexiraw FILE ALGORITHM SLACK --optional [NSIMS VERBOSITY]" << endl;
        exit(0);
    }

    mdplib_debug = true;

    double slack = atof(args[3]);
    int verbosity = argc > 5 ? atoi(args[5]) : 1;

    RawFileMOProblem* problem = new RawFileMOProblem(args[1]);
    problem->slack(slack);
    problem->generateAll();

    vector<Heuristic*> heuristics;
    heuristics.push_back(nullptr);
    heuristics.push_back(nullptr);
    problem->heuristics(heuristics);

    cerr << problem->states().size() << " states" << endl;

    clock_t startTime = clock();
    double tol = 1.0e-6;
    if (strcmp(args[2], "lao") == 0) {
        MOLAOStarSolver lao(problem, tol, 1000000);
        lao.solve(problem->initialState());
    } else if (strcmp(args[2], "vi") == 0) {
        LexiVISolver vi(problem, 1000000000, tol);
        vi.solve();
    }
    clock_t endTime = clock();
    if (verbosity > 0) {
        cerr << "Estimated cost "
             << ((MOState *) problem->initialState())->mobjCost()[0] << " "
             << ((MOState *) problem->initialState())->mobjCost()[1] << endl;
        cerr << "Time " << ((endTime - startTime + 0.0) / CLOCKS_PER_SEC) << endl;
    } else {
        cerr << ((MOState *) problem->initialState())->mobjCost()[0] << " "
             << ((MOState *) problem->initialState())->mobjCost()[1] << endl;
    }

    int nsims = argc > 4 ? atoi(args[4]) : 1;
    vector <double> expectedCost(2, 0.0);
    for (int i = 0; i < nsims; i++) {
        State* tmp = problem->initialState();
        if (verbosity > 100) {
            cerr << " ********* Simulation Starts ********* " << endl;
        }
        while (!problem->goal(tmp)) {
            Action* a;
            a = tmp->bestAction();
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
}


