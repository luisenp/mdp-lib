#include <iostream>
#include <vector>

#include "../include/solvers/solver.h"
#include "../include/solvers/LexiVISolver.h"
#include "../include/solvers/LexiLAOStarSolver.h"

#include "../include/util/general.h"

#include "../include/domains/gridworld/GridWorldState.h"
#include "../include/domains/gridworld/GridWorldAction.h"

#include "../include/lexi/lexi_problem.h"
#include "../include/lexi/domains/LexiGridWorldProblem.h"
#include "../include/lexi/domains/LexiGWManhattanHeuristic.h"

using namespace std;
using namespace mlcore;
using namespace mllexi;
using namespace mlsolvers;

int main(int argc, char* args[])
{
    if (argc <= 3) {
        cerr << "Usage ./textlexigw N SLACK ALGORITHM --optional [NSIMS VERBOSITY]" << endl;
        exit(0);
    }

    mdplib_debug = false;

    int n = atoi(args[1]);
    double slack = atof(args[2]);
    vector<PairDoubleMap> goals(1);
    goals[0].insert(make_pair(pair<int,int> (n - 1, n / 2), 0.0));
    goals[0].insert(make_pair(pair<int,int> (1, n - 1), 0.0));

    LexiGridWorldProblem* problem = new LexiGridWorldProblem(n, n, n-1, n-1, goals, 2, 1.0);
    problem->slack(slack);

    vector<Heuristic*> heuristics;
    heuristics.push_back(new LexiGWManhattanHeuristic(problem, 1.0));
    heuristics.push_back(new LexiGWManhattanHeuristic(problem, COST_DOWN_2));
    problem->heuristics(heuristics);
    GridWorldState* gws = (GridWorldState *) problem->initialState();

    LexiLAOStarSolver lao(problem, 0.0001, 10000000L);
    LexiVISolver vi(problem);

    if (strcmp(args[3], "lao") == 0) {
        dprint2("SOLVING WITH LAO", problem->initialState());
        lao.solve(problem->initialState());
        dprint1("SOLVED!");
    } else if (strcmp(args[3], "vi") == 0) {
        vi.solve();
    }
    cerr << "Estimated cost "
         << ((LexiState *) problem->initialState())->lexiCost()[0] << " "
         << ((LexiState *) problem->initialState())->lexiCost()[1] << endl;


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
            expectedCost[0] += problem->cost(tmp, a, 0);
            expectedCost[1] += problem->cost(tmp, a, 1);
            tmp = randomSuccessor(problem, tmp, a);
        }
        if (verbosity > 100)
            cerr << endl;
    }

    cerr << "Avg. Exec cost " << expectedCost[0] / nsims << " " << expectedCost[1] / nsims << endl;

    delete problem;
}

