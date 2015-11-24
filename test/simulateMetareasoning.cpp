#include <iostream>
#include <ctime>
#include <fstream>
#include <sstream>
#include <unistd.h>

#include "../include/solvers/solver.h"
#include "../include/solvers/MetareasoningSimulator.h"

#include "../include/util/general.h"
#include "../include/util/graph.h"

#include "../include/domains/gridworld/GridWorldProblem.h"
#include "../include/domains/gridworld/GridWorldState.h"
#include "../include/domains/gridworld/GridWorldAction.h"
#include "../include/domains/gridworld/GWManhattanHeuristic.h"

using namespace mlcore;
using namespace mlsolvers;
using namespace std;

int main(int argc, char* args[])
{
    PairDoubleMap goals;
    int n = atoi(args[1]);
    goals.insert(make_pair(pair<int,int> (n - 1, n - 1), 0.0));
    Problem* problem = new GridWorldProblem(n, n, 0, 0, &goals, 1.0);
    problem->gamma(0.90);
    Heuristic* heuristic = new GWManhattanHeuristic((GridWorldProblem*) problem);
    problem->setHeuristic(heuristic);

    problem->generateAll();

    MetareasoningSimulator simulator(problem);
    simulator.precomputeAllExpectedPolicyCosts();
}


