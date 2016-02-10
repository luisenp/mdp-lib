#include <iostream>
#include <ctime>
#include <fstream>
#include <sstream>
#include <unistd.h>

#include "../include/solvers/mobj/MObjSolvers.h"
#include "../include/solvers/mobj/LexiVISolver.h"
#include "../include/solvers/mobj/MOLAOStarSolver.h"
#include "../include/solvers/Solver.h"

#include "../include/util/general.h"
#include "../include/util/graph.h"

#include "../include/mobj/domains/airplane/AirplaneProblem.h"
#include "../include/mobj/domains/airplane/AirplaneState.h"
#include "../include/mobj/domains/airplane/AirplaneAction.h"
#include "../include/mobj/domains/airplane/AirplaneHeuristic.h"

using namespace mdplib_mobj_solvers;
using namespace mlcore;
using namespace mlsolvers;
using namespace mlmobj;
using namespace std;

int main(int argc, char* args[])
{
    if (argc < 3) {
        cerr << "Usage ./testairplane FILE ALGORITHM --optional [NSIMS VERBOSITY]" << endl;
        exit(0);
    }

    mdplib_debug = false;

    int verbosity = argc > 4 ? atoi(args[4]) : 1;

    double points[5][2] = {{0,0},{-1,0},{1,0},{0,-1},{0,1}};
//    double points[4][2] = {{0,0},{1,0},{-1,0},{0,1}};
//    double points[4][2] = {{0,0},{0,2},{-1,2},{1,2}};
    int n = 5;
    vector< vector< double> > distances;
    for (int i = 0; i < n; i++) {
        distances.push_back(vector<double> (n));
        for (int j = 0; j < n; j++) {
            double dx = points[i][0] - points[j][0];
            double dy = points[i][1] - points[j][1];
            distances[i][j] = sqrt(dx*dx + dy*dy);
        }
    }
    vector<double> probs;
    probs.push_back(0.0);
    probs.push_back(0.2);
    probs.push_back(0.4);
    probs.push_back(0.6);
    probs.push_back(0.8);

    int np = 3;
    vector<int> initLoc(np);
    for (int i = 0; i < np; i++)
        initLoc[i] = i + 1;

    MOProblem* problem = new AirplaneProblem(1, distances, probs, initLoc);

    vector<Heuristic*> heuristics;
    heuristics.push_back(new AirplaneHeuristic((AirplaneProblem *) problem, 0));
    heuristics.push_back(new AirplaneHeuristic((AirplaneProblem *) problem, 1));
    problem->heuristics(heuristics);
    problem->generateAll();

    if (verbosity > 1)
        cerr << problem->states().size() << " states" << endl;

    clock_t startTime = clock();
    double tol = 1.0e-6;
    if (strcmp(args[2], "lao") == 0) {
        MOLAOStarSolver lao(problem, tol, 1000000);
        lao.solve(problem->initialState());
    } else if (strcmp(args[2], "vi") == 0) {
        LexiVISolver vi(problem, 1000000000, tol);
        vi.solve();

        if (mdplib_debug) {
            for (State* s : problem->states()) {
                MOState* ls = (MOState*) s;
                if (ls->mobjCost()[0] < heuristics[0]->cost(s))
                    dprint4(ls, " ",heuristics[0]->cost(s), " AT LEVEL 0");
                if (ls->mobjCost()[1] < heuristics[1]->cost(s))
                    dprint4(ls, " ",heuristics[1]->cost(s), " AT LEVEL 1");
            }
        }
    }
    clock_t endTime = clock();
    if (verbosity > 0) {
        cerr << "Estimated cost "
             << ((MOState *) problem->initialState())->mobjCost()[0] << " "
             << ((MOState *) problem->initialState())->mobjCost()[1] << endl;
        cerr << startTime << " " << endTime << endl;
        cerr << "Time " << ((endTime - startTime + 0.0) / CLOCKS_PER_SEC) << endl;
    } else {
        cerr << ((MOState *) problem->initialState())->mobjCost()[0] << " "
             << ((MOState *) problem->initialState())->mobjCost()[1] << endl;
    }

    int nsims = argc > 5 ? atoi(args[3]) : 1;
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
                mdplib_debug = true;
                lexiBellmanUpdate(problem, lex, 0);
                mdplib_debug = false;
                double c0 = problem->cost(lex,a,0), c1 = problem->cost(lex,a,1);
                cerr << lex->mobjCost()[0] << " " <<  lex->mobjCost()[1];
                cerr << " - costs " << c0 << " " << c1 << endl;
            }

            expectedCost[0] += problem->cost(tmp, a, 0);
            expectedCost[1] += problem->cost(tmp, a, 1);
            tmp = randomSuccessor(problem, tmp, a);

            if (verbosity > 100)
                cerr << endl << "     NEXT *** " << tmp << endl;
        }
        if (verbosity > 100)
            cerr << endl;
    }

    if (verbosity > 0)
        cerr << "Avg. Exec cost " << expectedCost[0] / nsims << " " << expectedCost[1] / nsims << endl;

    delete problem;
}


