#include <iostream>
#include <ctime>
#include <fstream>
#include <sstream>
#include <unistd.h>

#include "../include/solvers/solver.h"
#include "../include/solvers/metareasoning/MetareasoningSimulator.h"

#include "../include/util/flags.h"
#include "../include/util/general.h"
#include "../include/util/graph.h"

#include "../include/domains/gridworld/GridWorldProblem.h"
#include "../include/domains/gridworld/GridWorldState.h"
#include "../include/domains/gridworld/GridWorldAction.h"
#include "../include/domains/gridworld/GWManhattanHeuristic.h"

using namespace mlcore;
using namespace mlsolvers;
using namespace std;
using namespace mdplib;

void
simulateAndPrintExpectedCosts(MetareasoningSimulator& simulator, int numSims)
{
    double expectedCost = 0.0;
    double expectedNOPCost = 0.0;
    for (int i = 0; i < numSims; i++) {
        pair <double, double> simResult = simulator.simulate();
        expectedCost += simResult.first;
        expectedNOPCost += simResult.second;
    }
    cout << expectedCost / numSims << " " <<
        expectedNOPCost / numSims << endl;
}

int main(int argc, char* args[])
{
    // Parsing flags
    register_flags(argc, args);
    if (!flag_is_registered_with_value("grid")) {
        cerr << "Must specify grid file using --grid" <<
            "=file command line flag." << endl;
        return -1;
    }
    string gridFile = flag_value("grid");
    string metaChoice = "none";
    int maxSteps = -1;
    if (flag_is_registered_with_value("meta"))
        metaChoice = flag_value("meta");
    bool tryAllActions = flag_is_registered("all_actions");
    int verbosity = 1;
    if (flag_is_registered_with_value("v"))
        verbosity = atoi(flag_value("v").c_str());
    int numSims = 1000;
    if (flag_is_registered_with_value("nsims"))
        numSims = atoi(flag_value("nsims").c_str());
    int numPlanningStepsPerAction = 1;
    if (flag_is_registered_with_value("steps_action"))
        numPlanningStepsPerAction = atoi(flag_value("steps_action").c_str());
    int numPlanningStepsPerNOP = 1;
    if (flag_is_registered_with_value("steps_nop"))
        numPlanningStepsPerNOP = atoi(flag_value("steps_nop").c_str());
    if (flag_is_registered_with_value("steps_range"))
        maxSteps = atoi(flag_value("steps_range").c_str());

    PairDoubleMap goals;
    Problem* problem = new GridWorldProblem(gridFile.c_str(), &goals);
    problem->gamma(0.99);
    Heuristic* heuristic =
        new GWManhattanHeuristic((GridWorldProblem*) problem);
    problem->setHeuristic(heuristic);
    problem->generateAll();

    MetareasoningSimulator simulator(problem);
    simulator.numPlanningStepsPerAction(numPlanningStepsPerAction);
    simulator.numPlanningStepsPerNOP(numPlanningStepsPerNOP);
    if (metaChoice.compare("assume1") == 0) {
        simulator.rule(META_ASSUMPTION_1);
    } else if (metaChoice.compare("assume2") == 0) {
        simulator.rule(META_ASSUMPTION_2);
    } else if (metaChoice.compare("change") == 0) {
        simulator.rule(META_CHANGE_ACTION);
    } else if (metaChoice.compare("nometa") == 0) {
        simulator.rule(NO_META);
    } else if (metaChoice.compare("optimal") == 0) {
        simulator.rule(OPTIMAL);
    } else if (metaChoice.compare("qvimprov") == 0) {
        simulator.rule(QVALIMPROV);
    }
    if (tryAllActions)
        simulator.tryAllActions(true);

    mdplib_debug = (verbosity > 1000);
    if (maxSteps == -1) {
        simulateAndPrintExpectedCosts(simulator, numSims);
    } else {
        for (int steps = 1; steps <= maxSteps; steps++) {
            simulator.numPlanningStepsPerAction(steps);
            simulator.numPlanningStepsPerNOP(steps);
            simulateAndPrintExpectedCosts(simulator, numSims);
        }
    }

    delete problem;
    delete heuristic;
}


