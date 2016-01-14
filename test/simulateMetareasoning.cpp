#include <ctime>
#include <fstream>
#include <iostream>
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
#include "../include/domains/racetrack/RacetrackProblem.h"
#include "../include/domains/racetrack/RTrackDetHeuristic.h"

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


vector<string> split(string s, char delim)
{
    vector<string> elems;
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim))
        elems.push_back(item);
    return elems;
}


void assignMetaRule(MetareasoningSimulator& simulator, string rule)
{
    if (rule.compare("assume1") == 0) {
        simulator.rule(META_ASSUMPTION_1);
    } else if (rule.compare("assume2") == 0) {
        simulator.rule(META_ASSUMPTION_2);
    } else if (rule.compare("change") == 0) {
        simulator.rule(META_CHANGE_ACTION);
    } else if (rule.compare("nometa") == 0) {
        simulator.rule(NO_META);
    } else if (rule.compare("optimal") == 0) {
        simulator.rule(OPTIMAL);
    } else if (rule.compare("qvimprov") == 0) {
        simulator.rule(QVALIMPROV);
    } else {
        assert(false);
    }
}


int main(int argc, char* args[])
{
    // Parsing flags
    register_flags(argc, args);
    if (!flag_is_registered_with_value("input")) {
        cerr << "Must specify input file using --input" <<
            "=file command line flag." << endl;
        return -1;
    }
    if (!flag_is_registered_with_value("domain")) {
        cerr << "Must specify domain using --domain" <<
            "=file command line flag." << endl;
        return -1;
    }
    string inputFile = flag_value("input");
    string domain = flag_value("domain");
    string metaRules = "none";
    int maxSteps = -1;
    if (flag_is_registered_with_value("rule"))
        metaRules = flag_value("rule");
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

    // Creating the domain
    PairDoubleMap goals;
    Problem* problem;
    Heuristic* heuristic;
    if (domain.compare("racetrack") == 0) {
        problem = new RacetrackProblem(inputFile.c_str());
        ((RacetrackProblem*) problem)->setPError(0.10);
        ((RacetrackProblem*) problem)->setPSlip(0.20);
        ((RacetrackProblem*) problem)->setMDS(-1);
        heuristic = nullptr;
    } else {
        problem = new GridWorldProblem(inputFile.c_str(), &goals);
        heuristic = new GWManhattanHeuristic((GridWorldProblem*) problem);
    }
    problem->gamma(0.99);
    problem->setHeuristic(heuristic);
    problem->generateAll();

    // Simulating
    MetareasoningSimulator simulator(problem);
    if (tryAllActions)
        simulator.tryAllActions(true);
    mdplib_debug = (verbosity > 1000);
    for (string rule : split(metaRules, ';')) {
        if (maxSteps == -1) {
            simulator.numPlanningStepsPerAction(numPlanningStepsPerAction);
            simulator.numPlanningStepsPerNOP(numPlanningStepsPerNOP);
            // This is here because OPTIMAL needs updated steps.
            assignMetaRule(simulator, rule);
            simulateAndPrintExpectedCosts(simulator, numSims);
        } else {
            for (int steps = 1; steps <= maxSteps; steps++) {
                simulator.numPlanningStepsPerAction(steps);
                simulator.numPlanningStepsPerNOP(steps);
                // This is here because OPTIMAL needs updated steps.
                assignMetaRule(simulator, rule);
                simulateAndPrintExpectedCosts(simulator, numSims);
            }
        }
    }

    delete problem;
    delete heuristic;
}


