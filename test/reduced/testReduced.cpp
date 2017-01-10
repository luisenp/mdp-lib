#include <cassert>
#include <ctime>
#include <list>
#include <sstream>
#include <string>
#include <typeinfo>
#include <vector>
#include <unordered_map>

#include "../../include/domains/racetrack/RacetrackProblem.h"
#include "../../include/domains/racetrack/RTrackDetHeuristic.h"
#include "../../include/domains/DummyState.h"
#include "../../include/domains/WrapperProblem.h"

#include "../include/ppddl/mini-gpt/states.h"
#include "../include/ppddl/mini-gpt/problems.h"
#include "../include/ppddl/mini-gpt/domains.h"
#include "../include/ppddl/mini-gpt/states.h"
#include "../include/ppddl/mini-gpt/exceptions.h"
#include "../../include/ppddl/PPDDLHeuristic.h"
#include "../../include/ppddl/PPDDLProblem.h"

#include "../../include/reduced/CustomReduction.h"
#include "../../include/reduced/LeastLikelyOutcomeReduction.h"
#include "../../include/reduced/MostLikelyOutcomeReduction.h"
#include "../../include/reduced/RacetrackObviousReduction.h"
#include "../../include/reduced/ReducedHeuristicWrapper.h"
#include "../../include/reduced/ReducedModel.h"
#include "../../include/reduced/ReducedState.h"
#include "../../include/reduced/ReducedTransition.h"

#include "../../include/solvers/LAOStarSolver.h"
#include "../../include/solvers/Solver.h"
#include "../../include/solvers/VISolver.h"

#include "../../include/util/flags.h"
#include "../../include/util/general.h"

#include "../../include/Problem.h"


using namespace std;
using namespace mdplib;
using namespace mlppddl;
using namespace mlreduced;
using namespace mlsolvers;


extern int yyparse();
extern FILE* yyin;
string current_file;
int warning_level = 0;

static int verbosity = 0;
static int k = 0;
double tau = 1.2;
int l = 2;

mlcore::Problem* problem = nullptr;
mlcore::Heuristic* heuristic = nullptr;
ReducedModel* reducedModel = nullptr;
ReducedHeuristicWrapper* reducedHeuristic = nullptr;
WrapperProblem* wrapperProblem = nullptr;
CustomReduction* bestReductionTemplate = nullptr;
list<ReducedTransition *> reductions;


/*
 * Given the input sizes = {s1, s2, ..., sn}, this function returns all possible
 * combinations of the sets {0, 1, ... s1 - 1}, {0, 1, ..., s2 - 1}, ...,
 * {0, 1, ..., sn - 1}, across the n sets.
 *
 * For example, sizes = {2, 2, 1} computes the following list of lists
 *  {0, 0, 0}, {0, 1, 0}, {1, 0, 0}, {1, 1, 0}.
 *
 * The result is stored in variable fullFactorialResult.
 */
void getFullFactorialIndices(vector<int> sizes,
                             list< list<int> > & fullFactorialResult) {
    if (sizes.size() == 1) {
        for (int i = 0; i < sizes[0]; i++) {
            fullFactorialResult.push_back(list<int> (1, i));
        }
        return;
    }
    getFullFactorialIndices(vector<int> (sizes.begin() + 1, sizes.end()),
                            fullFactorialResult);
    size_t prevSize = fullFactorialResult.size();
    for (int i = 0; i < sizes[0]; i++) {
        size_t j = 0;
        for (auto const oldCombination : fullFactorialResult) {
            list<int> newCombination(oldCombination);
            newCombination.push_front(i);
            fullFactorialResult.push_back(newCombination);
            if (++j == prevSize)
                break;

        }
    }
    for (size_t j = 0; j < prevSize; j++) {
        fullFactorialResult.pop_front();
    }
}


/*
 * Computes all possible (indices) combinations of l primary outcomes, across
 * actions groups of the specified sizes. The result is stored in variable
 * combinations.
 */
void getAllCombinations(vector<size_t>& groupSizes,
                             vector<vector<vector<int> > > & combinations) {
    // First we compute the possible primary outcomes combination for each
    // group
    vector < vector < vector<int> > > allCombinationsPrimaryGroups;
    vector<int> primaryCombSizes;
    for (size_t i = 0; i < groupSizes.size(); i++) {
        allCombinationsPrimaryGroups.push_back(vector <vector <int> > ());
        vector<int> currentCombination;
        for (size_t j = 0; j < l; j++)
            currentCombination.push_back(j);
        do {
            allCombinationsPrimaryGroups.back().push_back(
                vector<int> (currentCombination));
        } while (nextComb(currentCombination, groupSizes[i], l));
        primaryCombSizes.push_back(allCombinationsPrimaryGroups.back().size());
    }
//
//                                                                                for (int i = 0; i < allCombinationsPrimaryGroups.size(); i++) {
//                                                                                    for (int j = 0; j < allCombinationsPrimaryGroups[i].size(); j++) {
//                                                                                        for (int k = 0; k < allCombinationsPrimaryGroups[i][j].size(); k++) {
//                                                                                            std::cerr << allCombinationsPrimaryGroups[i][j][k] << " ";
//                                                                                        }
//                                                                                        std::cerr << ", ";
//                                                                                    }
//                                                                                    std::cerr << std::endl;
//                                                                                }


    // Now we get all combinations of indices across the combinations of
    // primary outcomes
    list<list <int> > fullIndicesCombination;
    getFullFactorialIndices(primaryCombSizes, fullIndicesCombination);
//                                                                                cerr << "****************************";
//                                                                                for (auto const xxx : fullIndicesCombination) {
//                                                                                    for (int yyy : xxx) {
//                                                                                        cerr << yyy << " ";
//                                                                                    }
//                                                                                    cerr << endl;
//                                                                                }
//                                                                                cerr << "****************************";

    // Now we compute the full set of combinations of primary outcomes
    // matching the full indices to the corresponding set of primary outcomes
    for (auto const combinationIndices : fullIndicesCombination) {
        vector < vector<int> > newCombination;
        int groupIdx = 0;
        for (int indexPrimarySet : combinationIndices) {
            newCombination.push_back(vector<int> (
                allCombinationsPrimaryGroups.at(groupIdx++)[indexPrimarySet]));
        }
        combinations.push_back(newCombination);
    }

//                                                                                for (int i = 0; i < combinations.size(); i++) {
//                                                                                    cerr << ": ";
//                                                                                    for (int j = 0; j < combinations[i].size(); j++) {
//                                                                                        for (int k = 0; k < combinations[i][j].size(); k++) {
//                                                                                            cerr << combinations[i][j][k] << " ";
//                                                                                        }
//                                                                                        cerr << ", ";
//                                                                                    }
//                                                                                    cerr << endl;
//                                                                                }
}


/*
 * This functions finds the best Mkl reduction, using brute force.
 */
void findBestReductionLOutcomes(mlcore::Problem* problem,
                             vector<vector<mlcore::Action*> > & actionGroups)
{
    // Getting all possible combination indices of l primary outcomes
    // across all action groups (each combination represents a reduction)
    vector<size_t> groupSizes;
    for (size_t i = 0; i < actionGroups.size(); i++) {
        groupSizes.push_back(
            bestReductionTemplate->
                primaryIndicatorsActions()[actionGroups[i][0]].size());

    }
    vector< vector < vector<int> > > reductions;
    getAllCombinations(groupSizes, reductions);
    // Evaluating all possible reductions
    double bestResult = mdplib::dead_end_cost + 1;
    const vector < vector<int> >* bestReduction;
    for (auto const & reduction : reductions ) {
        assert(reduction.size() == actionGroups.size());
        CustomReduction* testReduction =
            new CustomReduction(bestReductionTemplate);
        size_t groupIdx = 0;
        for (auto const primaryIndicesForGroup : reduction) {
            vector<mlcore::Action*> & actionGroup = actionGroups[groupIdx];
            unordered_map< mlcore::Action*, vector<bool> > &
                testPrimaryIndicatorsActions =
                    testReduction->primaryIndicatorsActions();
            for (mlcore::Action* a : actionGroup) {
                for (size_t j = 0;
                        j < testPrimaryIndicatorsActions[a].size();
                        j++) {
                    testPrimaryIndicatorsActions[a][j] = false;
                }
                for (auto const primaryIndex : primaryIndicesForGroup) {
                                                                                if (a == *(actionGroup.begin()))
                                                                                    cerr << primaryIndex << ",";
                    testPrimaryIndicatorsActions[a][primaryIndex] = true;
                }
            }
            groupIdx++;
                                                                                cerr << "; ";
        }
        reducedModel = new ReducedModel(problem, testReduction, k);
        double result = ReducedModel::evaluateMarkovChain(reducedModel);
//        double result = reducedModel->evaluateMonteCarlo(100);
                                                                                dprint1(result);
        if (result < bestResult) {
                                                                                dprint2("*********", result);
            bestResult = result;
            bestReduction = &reduction;
        }
    }

                                                                                dprint1("**************************");
    // Assigning the best primary outcomes found to the best reduction
    // template
    for (size_t groupIdx = 0; groupIdx < actionGroups.size(); groupIdx++) {
        const vector<int>& primaryIndicesForGroup = (*bestReduction)[groupIdx];
        vector<mlcore::Action*> & actionGroup = actionGroups[groupIdx];
        unordered_map< mlcore::Action*, vector<bool> > &
            primaryIndicatorsTempl =
                bestReductionTemplate->primaryIndicatorsActions();
        for (mlcore::Action* a : actionGroup) {
            for (size_t j = 0; j < primaryIndicatorsTempl[a].size(); j++) {
                primaryIndicatorsTempl[a][j] = false;
            }
            for (auto const primaryIndex : primaryIndicesForGroup) {
                                                                                if (a == *(actionGroup.begin()))
                                                                                    cerr << primaryIndex << ",";
                primaryIndicatorsTempl[a][primaryIndex] = true;
            }
        }
                                                                                cerr << "; ";
    }
    reducedModel = new ReducedModel(problem, bestReductionTemplate, k);
    double result = ReducedModel::evaluateMarkovChain(reducedModel);
                                                                                dprint2("best result", result);
}


/*
 * Finds the best reduction on the given problem using the greedy approach
 * described in http://anytime.cs.umass.edu/shlomo/papers/PZicaps14.pdf
 *
 * This methods receives a vector< vector<mlcore::Action> > parameter that
 * allows for the same reduction to be applied to multiple actions at the
 * same time. This is useful if the actions are symmetric, for example.
 */
void findBestReductionGreedy(mlcore::Problem* problem,
                             vector<vector<mlcore::Action*> > & actionGroups)
{
    // Evaluating expected cost of full reduction
    reducedModel = new ReducedModel(problem, bestReductionTemplate, k);
    double previousResult = ReducedModel::evaluateMarkovChain(reducedModel);
                                                                                dprint2("original", previousResult);
    int numOutcomes = 0;
    for (size_t i = 0; i < actionGroups.size(); i++)
        numOutcomes +=
            bestReductionTemplate->
                primaryIndicatorsActions()[actionGroups[i][0]].size();
                                                                                dprint2("num outcomes", numOutcomes);
    for (int i = 0; i < numOutcomes; i++) {
        double bestResult = mdplib::dead_end_cost + 1;
        int bestGroup = -1;
        int bestOutcomeIndex = -1;
        unordered_map< mlcore::Action*, vector <bool> > &
            primaryIndicatorsActions =
                bestReductionTemplate->primaryIndicatorsActions();
        for (size_t groupIdx = 0; groupIdx < actionGroups.size(); groupIdx++) {
            vector<mlcore::Action*> & actionGroup = actionGroups[groupIdx];
            int numPrimaryInGroup = 0;
            // Testing a new reduced model for each outcome of the actions in
            // this group. We use actionGroup[0] to get the number of outcomes,
            // because all actions in the same group should have the same
            // number of outcomes
            for (size_t outcomeIdx = 0;
                 outcomeIdx < primaryIndicatorsActions[actionGroup[0]].size();
                 outcomeIdx++) {
                // The greedy method works by removing outcomes. If it is
                // already removed, there is nothing to do
                if (!primaryIndicatorsActions[actionGroup[0]][outcomeIdx])
                    continue;
                CustomReduction* testReduction =
                    new CustomReduction(bestReductionTemplate);
                unordered_map< mlcore::Action*, vector<bool> > &
                    testPrimaryIndicatorsActions =
                        testReduction->primaryIndicatorsActions();
                // Remove this outcome from the set of primary outcomes for
                // all actions in the group
                for (mlcore::Action* a : actionGroup)
                    testPrimaryIndicatorsActions[a][outcomeIdx] = false;
                reducedModel = new ReducedModel(problem, testReduction, k);
                double result = ReducedModel::evaluateMarkovChain(reducedModel);
                if (result < bestResult) {
                    if (result < tau * previousResult) {
                        bestGroup = groupIdx;
                        bestOutcomeIndex = outcomeIdx;
                        bestResult = result;
                    }
                }
                // Set the outcome back to true to try a new model
                for (mlcore::Action* a : actionGroup)
                    testPrimaryIndicatorsActions[a][outcomeIdx] = false;
            }
        }
        // Checking if the current reduction satisfies the desired number of
        // primary outcomes (one of the stopping criteria)
        bool satisfiesL = true;
        for (size_t groupIdx = 0; groupIdx < actionGroups.size();
             groupIdx++) {
            vector<mlcore::Action*> & actionGroup = actionGroups[groupIdx];
            int primaryCount = 0;
            for (size_t outcomeIdx = 0;
                 outcomeIdx < primaryIndicatorsActions[actionGroup[0]].size();
                 outcomeIdx++) {
                if (primaryIndicatorsActions[actionGroup[0]][outcomeIdx])
                    primaryCount++;
            }
            if (primaryCount > l)
                satisfiesL = false;
        }
        // Checking if the decrement was too large to justify the new model
        // (the other stopping criterion)
        if (bestOutcomeIndex == -1) {
            // Couldn't find outcome to remove and maintain the cost
            // within the threshold
            break;
        } else {
            // Update best reduction/result and keep going.
            previousResult = bestResult;
            for (mlcore::Action* a : actionGroups[bestGroup]) {
                bestReductionTemplate->
                    primaryIndicatorsActions()[a][bestOutcomeIndex] = false;
            }
        }
        if (satisfiesL)
            break;
                                                                                dprint2("result ", bestResult);
    }

                                                                                unordered_map< mlcore::Action*, vector <bool> > &
                                                                                    primaryIndicatorsActions = bestReductionTemplate->primaryIndicatorsActions();
                                                                                for (size_t idx1 = 0; idx1 < actionGroups.size(); idx1++) {
                                                                                    cout << "group " << idx1 << ": ";
                                                                                    for (size_t idx2 = 0;
                                                                                         idx2 < primaryIndicatorsActions[actionGroups[idx1][0]].size();
                                                                                         idx2++) {
                                                                                        cout << primaryIndicatorsActions[actionGroups[idx1][0]][idx2] << " ";
                                                                                    }
                                                                                    cout << endl;
                                                                                }
                                                                                dprint2("greedy", previousResult);
}


void createRacetrackReductionsTemplate(RacetrackProblem* rtp)
{
    CustomReduction* reductionsTemplate = new CustomReduction(rtp);
    vector<bool> primaryIndicators;
    bool first = true;
    for (mlcore::Action* a : rtp->actions()) {
        // Setting primary outcomes for initial state
        if (first) {
            // All action work the same for this state, choose the first
            for (auto const & successor :
                 rtp->transition(rtp->initialState(), a)) {
                primaryIndicators.push_back(true);
            }
            reductionsTemplate->setPrimaryForState(
                rtp->initialState(), primaryIndicators);
            first = false;
        }
        primaryIndicators.clear();
        RacetrackAction* rta = static_cast<RacetrackAction*> (a);
        int numSuccessors = rtp->numSuccessorsAction(rta);
        for (int i = 0; i < numSuccessors; i++)
            primaryIndicators.push_back(true);
        reductionsTemplate->setPrimaryForAction(a, primaryIndicators);
    }
    reductions.push_back(reductionsTemplate);
    bestReductionTemplate = reductionsTemplate;
}


void initRacetrack(string trackName, int mds, double pslip, double perror)
{
    problem = new RacetrackProblem(trackName.c_str());
    static_cast<RacetrackProblem*>(problem)->pError(perror);
    static_cast<RacetrackProblem*>(problem)->pSlip(pslip);
    static_cast<RacetrackProblem*>(problem)->mds(mds);
    heuristic = new RTrackDetHeuristic(trackName.c_str());
    static_cast<RacetrackProblem*>(problem)->useFlatTransition(true);
    problem->generateAll();
    if (verbosity > 100)
        cout << "Generated " << problem->states().size() << " states." << endl;
    reductions.push_back(new LeastLikelyOutcomeReduction(problem));
    createRacetrackReductionsTemplate(static_cast<RacetrackProblem*> (problem));
}


int main(int argc, char* args[])
{
    register_flags(argc, args);
    if (flag_is_registered("debug"))
        mdplib_debug = true;

    // Reading flags.
    assert(flag_is_registered_with_value("domain"));
    string domainName = flag_value("domain");

    assert(flag_is_registered_with_value("problem"));

    if (flag_is_registered_with_value("v"))
        verbosity = stoi(flag_value("v"));

    if (flag_is_registered_with_value("k"))
        k = stoi(flag_value("k"));

    if (flag_is_registered_with_value("l"))
        l = stoi(flag_value("l"));

    int nsims = 100;
    if (flag_is_registered_with_value("n"))
        nsims = stoi(flag_value("n"));

    // Creating problem
    vector< vector<mlcore::Action*> >
    actionGroups(3, vector<mlcore::Action*> ());
    if (domainName == "racetrack") {
        int mds = -1;
        double perror = 0.05;
        double pslip = 0.10;
        if (flag_is_registered_with_value("mds"))
            mds = stoi(flag_value("mds"));
        if (flag_is_registered_with_value("pslip"))
            pslip = stoi(flag_value("pslip"));
        if (flag_is_registered_with_value("perror"))
            perror = stoi(flag_value("perror"));
        string trackName = flag_value("problem");
        initRacetrack(trackName, mds, pslip, perror);
        // Actions with the same magnitude will be considered part of the
        // same group. Then the same reduction will be applied to all actions
        // in the same group
        for (mlcore::Action* a : problem->actions()) {
            RacetrackAction* rta = static_cast<RacetrackAction*> (a);
            int magnitude = abs(rta->ax()) + abs(rta->ay());
            actionGroups[magnitude].push_back(a);
        }
    }

    bool useFullTransition = flag_is_registered("use_full");

    ReducedTransition* bestReduction = nullptr;
    bestReduction = reductions.front();
    wrapperProblem = new WrapperProblem(problem);

    // Finding the best reduction using Monte Carlo simulations
    double totalPlanningTime = 0.0;
    mlcore::StateSet reachableStates, tipStates;
    getReachableStates(problem, reachableStates, tipStates, 4);
//    wrapperProblem->overrideGoals(&tipStates);
//    cout << "reachable/tip states: " << reachableStates.size() <<
//        "/" << tipStates.size() << endl;
    clock_t startTime = clock();
    // Finds the best reduction using the greedy approach and then stores it
    // in global variable bestReductionTemplate
//    findBestReductionGreedy(wrapperProblem, actionGroups);
    findBestReductionLOutcomes(wrapperProblem, actionGroups);
    clock_t endTime = clock();
                                                                                dprint1("found best reduction greedy");

//    for (auto const & reduction : reductions) {
//        reducedModel = new ReducedModel(wrapperProblem, reduction, k);
//        double result = reducedModel->evaluateMonteCarlo(20);
//        cout << result << endl;
//    }

    double timeReductions = double(endTime - startTime) / CLOCKS_PER_SEC;
    totalPlanningTime += timeReductions;
    cout << "time finding reductions " << timeReductions << endl;

    // Setting up the final reduced model to use
    reducedModel = new ReducedModel(problem, bestReductionTemplate, k);
    reducedHeuristic = new ReducedHeuristicWrapper(heuristic);
    reducedModel->setHeuristic(reducedHeuristic);
    static_cast<ReducedModel*>(reducedModel)->
        useFullTransition(useFullTransition);

    // We will now use the wrapper for the pro-active re-planning approach. It
    // will allow us to plan in advance for the set of successors of a
    // state-action
    wrapperProblem->clearOverrideGoals();
    wrapperProblem->setNewProblem(reducedModel);

    // Solving reduced model using LAO*
    startTime = clock();
    LAOStarSolver solver(wrapperProblem);
    solver.solve(wrapperProblem->initialState());
    endTime = clock();
    double timeInitialPlan = (double(endTime - startTime) / CLOCKS_PER_SEC);
    totalPlanningTime += timeInitialPlan;
    cout << "cost " << wrapperProblem->initialState()->cost() <<
        " time " << timeInitialPlan << endl;


    // Running a trial of the continual planning approach.
    double expectedCost = 0.0;
    for (int i = 0; i < nsims; i++) {
        pair<double, double> costAndTime =
            reducedModel->trial(solver, wrapperProblem);
        expectedCost += costAndTime.first;
    }
    cout << "expected cost " << expectedCost / nsims << endl;
    cout << "total planning time " << totalPlanningTime << endl;

    // Releasing memory
    for (auto reduction : reductions)
        delete reduction;
    reducedModel->cleanup();
    delete reducedModel;
    wrapperProblem->cleanup();
    delete wrapperProblem;
    delete problem;
    return 0;
}
