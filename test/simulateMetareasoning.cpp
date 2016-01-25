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
    } else if (rule.compare("multnop") == 0) {
        simulator.rule(MULTNOP);
    } else {
        assert(false);
    }
}

int main(int argc, char* args[])
{
    // Parsing flags.
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
    bool costNOPEqualsStepNOP = false;
    if (flag_is_registered("same_cost_nop"))
        costNOPEqualsStepNOP = true;
    if (flag_is_registered_with_value("rule"))
        metaRules = flag_value("rule");
    bool tryAllActions = flag_is_registered("all_actions");
    int verbosity = 1;
    if (flag_is_registered_with_value("v"))
        verbosity = atoi(flag_value("v").c_str());
    int numSims = 1000;
    if (flag_is_registered_with_value("nsims"))
        numSims = atoi(flag_value("nsims").c_str());
    if (verbosity > 1000) {
        for (int i = 1; i < argc; i++)
            cout << args[i] << endl;
    }

    /*
     * The user can specify a list of time steps duration for actions and NOP.
     * The experiment will traverse these lists and take pairs of values
     * at the same index, and create a experiment for each of those pairs.
     * If one list is shorter than the other, the last value will be use for
     * all remaining experiments.
     */
    vector<int> allStepsAction;
    if (flag_is_registered_with_value("steps_action")) {
        for (string step : split(flag_value("steps_action"), ';'))
            allStepsAction.push_back(atoi(step.c_str()));
    }
    vector<int> allStepsNOP;
    if (flag_is_registered_with_value("steps_nop")) {
        for (string step : split(flag_value("steps_nop"), ';'))
            allStepsNOP.push_back(atoi(step.c_str()));
    }

    // Creating the domain.
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

    // Simulating.
    MetareasoningSimulator simulator(problem);
    if (tryAllActions)
        simulator.tryAllActions(true);
    mdplib_debug = (verbosity > 1000);
    for (string rule : split(metaRules, ';')) {
        for (int i = 0;
             i < max(allStepsNOP.size(), allStepsAction.size());
             i++) {
            int stepActionIndex = min(i, (int) allStepsAction.size() - 1);
            int stepNOPIndex = min(i, (int) allStepsNOP.size() - 1);
            simulator.numPlanningStepsPerAction(allStepsAction[stepActionIndex]);
            simulator.numPlanningStepsPerNOP(allStepsNOP[stepNOPIndex]);
            if (costNOPEqualsStepNOP)
                simulator.costNOP(allStepsNOP[stepNOPIndex]);

            // This is here because OPTIMAL calculation needs updated steps,
            // and the calculation happens when the rule is assigned.
            assignMetaRule(simulator, rule);
            simulateAndPrintExpectedCosts(simulator, numSims);
        }
    }

    delete problem;
    delete heuristic;
}


