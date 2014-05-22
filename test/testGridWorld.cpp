#include <iostream>

#include "../include/util/general.h"
#include "../include/util/rational.h"
#include "../include/domains/gridworld/GridWorldState.h"
#include "../include/domains/gridworld/GridWorldProblem.h"
#include "../include/domains/gridworld/GridWorldAction.h"

using namespace std;

int main()
{
    PairRationalMap goals;
    goals.insert(make_pair(pair<int,int> (2,2), Rational(1)));

    GridWorldProblem problem(3, 3, 0, 0, &goals);
    GridWorldState* gws = (GridWorldState *) problem.initialState();

    problem.generateAll();
    StateSet states = problem.states();
    std::cout << states.size() << std::endl;
}
