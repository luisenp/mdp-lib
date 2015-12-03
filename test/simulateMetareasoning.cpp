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
    if (argc < 5) {
        cout << "Usage: ./simulmeta MAP METHOD TRYALLACTIONS NSIMS" << endl;
        return 0;
    }
    PairDoubleMap goals;
    Problem* problem = new GridWorldProblem(args[1], &goals);
    problem->gamma(0.90);
    Heuristic* heuristic =
        new GWManhattanHeuristic((GridWorldProblem*) problem);
    problem->setHeuristic(heuristic);
    problem->generateAll();

    MetareasoningSimulator simulator(problem);
    if (strcmp(args[2], "assume1") == 0)
        simulator.rule(META_ASSUMPTION_1);
    if (strcmp(args[2], "assume2") == 0)
        simulator.rule(META_ASSUMPTION_2);
    if (strcmp(args[2], "nometa") == 0)
        simulator.rule(NO_META);
    if (strcmp(args[3], "true") == 0)
        simulator.tryAllActions(true);

    int numSims = atoi(args[4]);
    double expectedCost = 0.0;
    double expectedNOPCost = 0.0;
    for (int i = 0; i < numSims; i++) {
        pair <double, double> simResult = simulator.simulate();
        expectedCost += simResult.first;
        expectedNOPCost += simResult.second;
    }

    cout << "Expected Cost " << expectedCost / numSims << endl;
    cout << "Expected NOP Cost " << expectedNOPCost / numSims << endl;

    delete problem;
    delete heuristic;
}


