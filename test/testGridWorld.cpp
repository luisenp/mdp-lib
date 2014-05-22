#include <iostream>

#include "../include/solvers/VISolver.h"
#include "../include/util/general.h"
#include "../include/util/rational.h"
#include "../include/domains/gridworld/GridWorldState.h"
#include "../include/domains/gridworld/GridWorldProblem.h"
#include "../include/domains/gridworld/GridWorldAction.h"

using namespace std;

int main()
{
    PairRationalMap goals;
    goals.insert(make_pair(pair<int,int> (2,2), Rational(-1)));

    Problem* problem = new GridWorldProblem(3, 3, 0, 0, &goals);
    GridWorldState* gws = (GridWorldState *) problem->initialState();

    problem->generateAll();
    problem->generateAll();
    StateSet states = problem->states();

    VISolver solver(problem);
    solver.solve(10);
    delete ((GridWorldProblem *) problem);
}
