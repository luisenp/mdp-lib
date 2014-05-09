#include <iostream>

#include "../include/util/general.h"
#include "../include/util/rational.h"
#include "../include/domains/gridworld/GridWorldState.h"
#include "../include/domains/gridworld/GridWorldProblem.h"
#include "../include/domains/gridworld/GridWorldAction.h"

int main()
{
    IntPairSet goals;
    goals.insert(std::pair<int,int> (9,9));
    GridWorldProblem problem(10, 10, 0, 0, &goals);
    const GridWorldState* gws = (GridWorldState *) problem.getInitialState();
    std::cout << gws << std::endl;
}
