#include <iostream>

#include "../include/util/general.h"
#include "../include/util/rational.h"
#include "../include/domains/gridworld/GridWorldState.h"
#include "../include/domains/gridworld/GridWorldProblem.h"
#include "../include/domains/gridworld/GridWorldAction.h"

using namespace std;

int main()
{
    IntPairSet goals;
    goals.insert(pair<int,int> (9,9));
    GridWorldProblem problem(10, 10, 0, 0, &goals);
    GridWorldState* gws = (GridWorldState *) problem.getInitialState();

    problem.generateAll();
    StateSet states = problem.getStates();
    std::cout << states.size() << std::endl;
}
