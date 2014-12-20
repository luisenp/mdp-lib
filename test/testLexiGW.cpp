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
    int n = atoi(args[1]);
    double slack = atof(args[2]);
    vector<PairDoubleMap> goals(1);
    goals[0].insert(make_pair(pair<int,int> (n - 5, n - 1), 0.0));
    goals[0].insert(make_pair(pair<int,int> (n - 1, n - 4), 0.0));

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

    mdplib_debug = true;
    LexiState* s = (LexiState*) problem->initialState();
    dprint3(s, s->lexiCost()[0], s->lexiCost()[1]);
    for (State* state : problem->states()) {
        LexiState* s = (LexiState*) state;
        if (heuristics[0]->cost(s) > s->lexiCost()[0])
            dprint4("ERROR ", s, heuristics[0]->cost(s), s->lexiCost()[0]);
    }
//    while (true) {
//        dprint3(s, s->lexiCost()[0], s->lexiCost()[1]);
//        dprint2(heuristics[0]->cost(s), heuristics[1]->cost(s));
//        if (problem->goal(s, 0))
//            break;
//        dprint1(s->bestAction());
//        s = (LexiState* ) randomSuccessor(problem, s, s->bestAction());
//    }

    delete problem;
}

