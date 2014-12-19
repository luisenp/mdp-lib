#include <iostream>
#include <vector>

#include "../include/solvers/solver.h"
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
    goals[0].insert(make_pair(pair<int,int> (0, n - 1), 0.0));
    goals[0].insert(make_pair(pair<int,int> (n - 1, 3 * n / 4), 0.0));

    LexiGridWorldProblem* problem = new LexiGridWorldProblem(n, n, n-1, n-1, goals, 2, 1.0);
    problem->slack(slack);

    vector<Heuristic*> heuristics;
    heuristics.push_back(new LexiGWManhattanHeuristic(problem, 1.0));
    heuristics.push_back(new LexiGWManhattanHeuristic(problem, COST_DOWN_2));
    problem->heuristics(heuristics);
    GridWorldState* gws = (GridWorldState *) problem->initialState();

    LexiLAOStarSolver lao(problem, 0.0001, 10000000L);

    mdplib_debug = true;
    dprint1(problem->states().size());
    lao.solve(problem->initialState());
    dprint1(problem->states().size());
    dprint1(lao.explicitGraph_.size());

    LexiState* s = (LexiState*) problem->initialState();
    while (true) {
        dprint3(s, s->lexiCost()[0], s->lexiCost()[1]);
        dprint2(heuristics[0]->cost(s), heuristics[1]->cost(s));
        if (problem->goal(s, 0))
            break;
        dprint1(s->bestAction());
//        lexiBellmanUpdate(problem, s);
        s = (LexiState* ) randomSuccessor(problem, s, s->bestAction());
    }


    delete problem;
}

