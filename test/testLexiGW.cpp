#include <iostream>
#include <vector>

#include "../include/solvers/solver.h"
#include "../include/solvers/LexiLAOStarSolver.h"

#include "../include/util/general.h"

#include "../include/domains/gridworld/GridWorldState.h"
#include "../include/domains/gridworld/GridWorldAction.h"

#include "../include/lexi/lexi_problem.h"
#include "../include/lexi/domains/LexiGridWorldProblem.h"

using namespace std;
using namespace mlcore;
using namespace mllexi;
using namespace mlsolvers;

int main(int argc, char* args[])
{
    int n = 10;
    vector<PairDoubleMap> goals(1);
    goals[0].insert(make_pair(pair<int,int> (0, n - 1), 0.0));
    goals[0].insert(make_pair(pair<int,int> (n - 1, 0), 0.0));

    LexiProblem* problem = new LexiGridWorldProblem(n, n, n-1, n-1, goals, 2, 1.0);
    GridWorldState* gws = (GridWorldState *) problem->initialState();

    LexiLAOStarSolver lao(problem, 0.0001, 10000000L);

    lao.solve(problem->initialState());

    LexiState* s = (LexiState*) problem->initialState();
    while (true) {
        dprint4(s, s->bestAction(), s->lexiCost()[0], s->lexiCost()[1]);
        if (problem->goal(s, 0))
            break;
        s = (LexiState* ) randomSuccessor(problem, s, s->bestAction());
    }


    delete problem;
}

