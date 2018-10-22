#include <cassert>
#include <cfloat>
#include <ctime>
#include <list>
#include <sstream>
#include <string>
#include <typeinfo>
#include <vector>
#include <unordered_map>

#include "../../include/domains/racetrack/RacetrackProblem.h"
#include "../../include/domains/racetrack/RTrackDetHeuristic.h"
#include "../../include/domains/sailing/SailingProblem.h"
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

#include "../../include/solvers/AODetHeuristic.h"
#include "../../include/solvers/LAOStarSolver.h"
#include "../../include/solvers/LRTDPSolver.h"
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
double tau = 1.1;
int l = 1;
bool isDeterminization = false;
long maxt = -1;

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

    // Now we get all combinations of indices across the combinations of
    // primary outcomes
    list<list <int> > fullIndicesCombination;
    getFullFactorialIndices(primaryCombSizes, fullIndicesCombination);

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
}


/*
 * Assigns the set of primary outcomes to use for each action group to the
 * given reduction.
 */
void assignPrimaryOutcomesToReduction(
    const vector<vector<int> > & primaryOutcomesForGroups,
    const vector<vector<mlcore::Action*> > & actionGroups,
    CustomReduction* reduction)
{
    for (size_t groupIdx = 0; groupIdx < actionGroups.size(); groupIdx++) {
        const vector<int>& primaryIndicesForGroup =
            primaryOutcomesForGroups[groupIdx];
        const vector<mlcore::Action*> & actionGroup = actionGroups[groupIdx];
        unordered_map< mlcore::Action*, vector<bool> > &
            primaryIndicatorsTempl =
                reduction->primaryIndicatorsActions();
        for (mlcore::Action* a : actionGroup) {
            for (size_t j = 0; j < primaryIndicatorsTempl[a].size(); j++) {
                primaryIndicatorsTempl[a][j] = false;
            }
            for (auto const primaryIndex : primaryIndicesForGroup) {
                primaryIndicatorsTempl[a][primaryIndex] = true;
            }
        }
    }
}


/*
 * Prints the given reduction to standard output.
 */
void printCustomReduction(CustomReduction* reduction,
                          vector< vector<mlcore::Action*> >& actionGroups)
{
    unordered_map< mlcore::Action*, vector <bool> > &
        primaryIndActions = reduction->primaryIndicatorsActions();
    for (size_t idx1 = 0; idx1 < actionGroups.size(); idx1++) {
        cout << "group " << idx1 << ": ";
        for (size_t idx2 = 0;
             idx2 < primaryIndActions[actionGroups[idx1][0]].size(); idx2++) {
            cout << primaryIndActions[actionGroups[idx1][0]][idx2] << " ";
        }
        cout << endl;
    }
}


/*
 * This functions finds the best Mkl reduction, using brute force.
 */
void findBestReductionBruteForce(
    mlcore::Problem* problem, vector<vector<mlcore::Action*> > & actionGroups)
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
        assignPrimaryOutcomesToReduction(reduction,
                                         actionGroups,
                                         testReduction);
        reducedModel = new ReducedModel(problem, testReduction, k);
        double result = ReducedModel::evaluateMarkovChain(reducedModel);
                                                                                dprint(result);
        if (result < bestResult) {
                                                                                dprint("*********", result);
                                                                                if (mdplib_debug)
                                                                                    printCustomReduction(testReduction, actionGroups);
            bestResult = result;
            bestReduction = &reduction;
        }
    }
                                                                                dprint("**************************");
    assignPrimaryOutcomesToReduction(*bestReduction,
                                     actionGroups,
                                     bestReductionTemplate);
    reducedModel = new ReducedModel(problem, bestReductionTemplate, k);
    double result = ReducedModel::evaluateMarkovChain(reducedModel);
                                                                                dprint("best result", result);
                                                                                if (mdplib_debug)
                                                                                    printCustomReduction(bestReductionTemplate, actionGroups);
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
    double originalResult = ReducedModel::evaluateMarkovChain(reducedModel);
    double previousResult = originalResult;
                                                                                dprint("original", previousResult);
    int numOutcomes = 0;
    for (size_t i = 0; i < actionGroups.size(); i++)
        numOutcomes +=
            bestReductionTemplate->
                primaryIndicatorsActions()[actionGroups[i][0]].size();
                                                                                dprint("num outcomes", numOutcomes);
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
                if (result < mdplib::dead_end_cost && result < bestResult) {
                    bestGroup = groupIdx;
                    bestOutcomeIndex = outcomeIdx;
                    bestResult = result;
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

        // Printing best reduction
        if (verbosity > 100) {
            cout << "Best reduction so far: " << endl;
            printCustomReduction(bestReductionTemplate, actionGroups);
        }
        // Checking if there was an outcome that could be removed
        if (bestOutcomeIndex == -1) {
            // Couldn't find outcome to remove and result in solvable model
            break;
        } else {
            if ( (bestResult - originalResult) > (originalResult * tau)
                    && satisfiesL )
                break;
            // Update best reduction/result and keep going.
            previousResult = bestResult;
            for (mlcore::Action* a : actionGroups[bestGroup]) {
                bestReductionTemplate->
                    primaryIndicatorsActions()[a][bestOutcomeIndex] = false;
            }
        }
                                                                                dprint("result ", bestResult);

    }
    // Printing best reduction
    if (verbosity > 10) {
        printCustomReduction(bestReductionTemplate, actionGroups);
        cout << "Greedy found model with cost: " << previousResult << endl;

    }
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


void createSailingReductionsTemplate(SailingProblem* sp)
{
    CustomReduction* reductionsTemplate = new CustomReduction(sp);
    vector<bool> primaryIndicators(8, true);
    bool first = true;
    for (mlcore::Action* a : sp->actions()) {
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
    if (flag_is_registered_with_value("heuristic")
            && flag_value("heuristic") == "aodet") {
        heuristic = new AODetHeuristic(problem);
    } else {
        heuristic = new RTrackDetHeuristic(trackName.c_str());
    }
    static_cast<RacetrackProblem*>(problem)->useFlatTransition(true);
    problem->generateAll();
    if (verbosity > 100)
        cout << "Generated " << problem->states().size() << " states." << endl;
    reductions.push_back(new LeastLikelyOutcomeReduction(problem));
    createRacetrackReductionsTemplate(static_cast<RacetrackProblem*> (problem));
}


void initSailing()
{
    static vector<double> costs;
    costs.push_back(1);
    costs.push_back(2);
    costs.push_back(5);
    costs.push_back(10);
    costs.push_back(mdplib::dead_end_cost + 1);

    static double windTransition[] = {
        0.20, 0.20, 0.20, 0.00, 0.00, 0.00, 0.20, 0.20,
        0.20, 0.20, 0.20, 0.20, 0.00, 0.00, 0.00, 0.20,
        0.20, 0.20, 0.20, 0.20, 0.20, 0.00, 0.00, 0.00,
        0.00, 0.20, 0.20, 0.20, 0.20, 0.20, 0.00, 0.00,
        0.00, 0.00, 0.20, 0.20, 0.20, 0.20, 0.20, 0.00,
        0.00, 0.00, 0.00, 0.20, 0.20, 0.20, 0.20, 0.20,
        0.20, 0.00, 0.00, 0.00, 0.20, 0.20, 0.20, 0.20,
        0.20, 0.20, 0.00, 0.00, 0.00, 0.20, 0.20, 0.20};

    if (!flag_is_registered_with_value("sailing-size")) {
        cerr << "Must specify sailing-size argument flag" << endl;
        exit(-1);
    }
    int sizeSailing = atoi(flag_value("sailing-size").c_str());

    if (!flag_is_registered_with_value("sailing-goal")) {
        cerr << "Must specify sailing-goal argument flag" << endl;
        exit(-1);
    }
    int goalSailing = atoi(flag_value("sailing-goal").c_str());

    if (verbosity > 100) {
        cout << "Setting up sailing domain with size " << sizeSailing <<
            " with goal " << goalSailing << endl;
    }

    problem = new SailingProblem(0, 0, 0,
                                 goalSailing, goalSailing,
                                 sizeSailing, sizeSailing,
                                 costs,
                                 windTransition,
                                 true);
    static_cast<SailingProblem*>(problem)->useFlatTransition(true);
    problem->generateAll();

    if (!flag_is_registered_with_value("heuristic") ||
            flag_value("heuristic") == "no-wind") {
        heuristic =
            new SailingNoWindHeuristic(static_cast<SailingProblem*>(problem));
    } else {
        heuristic = new AODetHeuristic(problem);
    }
    createSailingReductionsTemplate(static_cast<SailingProblem*> (problem));
}


int main(int argc, char* args[])
{
    register_flags(argc, args);
    if (flag_is_registered("debug"))
        mdplib_debug = true;

    // Reading flags.
    assert(flag_is_registered_with_value("domain"));
    string domainName = flag_value("domain");

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
    vector< vector<mlcore::Action*> > actionGroups;
    if (domainName == "racetrack") {
        assert(flag_is_registered_with_value("problem"));
        int mds = -1;
        double perror = 0.05;
        double pslip = 0.10;
        if (flag_is_registered_with_value("mds"))
            mds = stoi(flag_value("mds"));
        if (flag_is_registered_with_value("pslip"))
            pslip = stof(flag_value("pslip"));
        if (flag_is_registered_with_value("perror"))
            perror = stof(flag_value("perror"));
        string trackName = flag_value("problem");
        initRacetrack(trackName, mds, pslip, perror);
        // Actions with the same magnitude will be considered part of the
        // same group. Then the same reduction will be applied to all actions
        // in the same group
        for (int i = 0; i < 3; i++)
            actionGroups.push_back(vector<mlcore::Action*> ());
        for (mlcore::Action* a : problem->actions()) {
            RacetrackAction* rta = static_cast<RacetrackAction*> (a);
            int magnitude = abs(rta->ax()) + abs(rta->ay());
            actionGroups[magnitude].push_back(a);
        }
    } else if (domainName == "sailing") {
        initSailing();
        for (mlcore::Action* a : problem->actions()) {
            actionGroups.push_back(vector<mlcore::Action*> ());
            actionGroups.back().push_back(a);
        }
    }

    bool useFullTransition = flag_is_registered("use-full");

    ReducedTransition* bestReduction = nullptr;
    bestReduction = reductions.front();
    wrapperProblem = new WrapperProblem(problem);

    double totalPlanningTime = 0.0;
    mlcore::StateSet reachableStates, tipStates, subgoals;
    clock_t startTime = clock();
    if (flag_is_registered("use-subgoals")) {
        // This is experimental. Results are not encouraging
        int depth = 4;
        if (flag_is_registered_with_value("d"))
            depth = stoi(flag_value("d"));
        getReachableStates(problem, reachableStates, tipStates, depth);
        while (subgoals.size() < tipStates.size() / 10) {
            auto it = tipStates.begin();
            advance(it, rand() % (tipStates.size() - 1));
            subgoals.insert(*it);
        }
        wrapperProblem->overrideGoals(&subgoals);
    }
    if (flag_is_registered("use-brute-force")) {
        findBestReductionBruteForce(wrapperProblem, actionGroups);
    } else if (flag_is_registered("best-m02-racetrack-greedy")) {
        vector<vector<int> > primaryOutcomes;
        primaryOutcomes.push_back(vector<int>{1});
        primaryOutcomes.push_back(vector<int>{0,1});
        primaryOutcomes.push_back(vector<int>{1});
        assignPrimaryOutcomesToReduction(primaryOutcomes,
                                         actionGroups,
                                         bestReductionTemplate);
    } else if (flag_is_registered("best-det-racetrack-greedy")) {
        isDeterminization = true;
        vector<vector<int> > primaryOutcomes;
        primaryOutcomes.push_back(vector<int>{1});
        primaryOutcomes.push_back(vector<int>{1});
        primaryOutcomes.push_back(vector<int>{1});
        assignPrimaryOutcomesToReduction(primaryOutcomes,
                                         actionGroups,
                                         bestReductionTemplate);
    } else if (flag_is_registered("best-m02-sailing-greedy")) {
        // This was learned on a sailing of size 5, with goal at the corner
        vector<vector<int> > primaryOutcomes;
        primaryOutcomes.push_back(vector<int>{0});
        primaryOutcomes.push_back(vector<int>{0, 5});
        primaryOutcomes.push_back(vector<int>{4});
        primaryOutcomes.push_back(vector<int>{4});
        primaryOutcomes.push_back(vector<int>{3});
        primaryOutcomes.push_back(vector<int>{5});
        primaryOutcomes.push_back(vector<int>{5});
        primaryOutcomes.push_back(vector<int>{6});
        assignPrimaryOutcomesToReduction(primaryOutcomes,
                                         actionGroups,
                                         bestReductionTemplate);
    } else if (flag_is_registered("best-det-sailing-greedy")) {
        isDeterminization = true;
        // This was learned on a sailing of size 5, with goal at the corner
        vector<vector<int> > primaryOutcomes;
        primaryOutcomes.push_back(vector<int>{0});
        primaryOutcomes.push_back(vector<int>{0});
        primaryOutcomes.push_back(vector<int>{4});
        primaryOutcomes.push_back(vector<int>{4});
        primaryOutcomes.push_back(vector<int>{3});
        primaryOutcomes.push_back(vector<int>{5});
        primaryOutcomes.push_back(vector<int>{5});
        primaryOutcomes.push_back(vector<int>{6});
        assignPrimaryOutcomesToReduction(primaryOutcomes,
                                         actionGroups,
                                         bestReductionTemplate);
    } else if (flag_is_registered("greedy")) {
        // Finds the best reduction using the greedy approach and then stores
        // it in global variable bestReductionTemplate
        if (l == 1) isDeterminization = true;
        findBestReductionGreedy(wrapperProblem, actionGroups);
    }

    clock_t endTime = clock();
    double timeReductions = double(endTime - startTime) / CLOCKS_PER_SEC;
    totalPlanningTime += timeReductions;
    cout << "time finding reductions " << timeReductions << endl;

    // Setting up the final reduced model to use
    double kappa = DBL_MAX;
    if (flag_is_registered_with_value("kappa"))
        kappa = stof(flag_value("kappa"));
    reducedModel = new ReducedModel(problem, bestReductionTemplate, k, tau);
    reducedHeuristic = new ReducedHeuristicWrapper(heuristic);
    reducedModel->setHeuristic(reducedHeuristic);
    reducedModel->useFullTransition(useFullTransition);
    reducedModel->increaseK(flag_is_registered("increase-k"));

    // We will now use the wrapper for the pro-active re-planning approach. It
    // will allow us to plan in advance for the set of successors of a
    // state-action
    wrapperProblem->clearOverrideGoals();
    wrapperProblem->setNewProblem(reducedModel);

    // Solving off-line using full models
    Solver* solver;
    startTime = clock();
    if (flag_is_registered("use-vi")) {
        reducedModel->generateAll();
        solver = new VISolver(wrapperProblem);
    } else {
        if (flag_is_registered("use-lrtdp"))
            solver = new LRTDPSolver(wrapperProblem, 1000000, 1.0e-6, -1);
        else
            solver = new LAOStarSolver(wrapperProblem);
    }
    if (flag_is_registered_with_value("maxt"))
        maxt = stoi(flag_value("maxt"));
    solver->maxPlanningTime(maxt);
    if (useFullTransition)
        solver->solve(wrapperProblem->initialState());
    endTime = clock();
    double timeInitialPlan = (double(endTime - startTime) / CLOCKS_PER_SEC);
    totalPlanningTime += timeInitialPlan;
    cout << "cost " << wrapperProblem->initialState()->cost() <<
        " time " << timeInitialPlan << endl;

    // Running a trial of the continual planning approach.
    double expectedCost = 0.0;
    double expectedTime = 0.0;
    double maxReplanningTime = 0.0;
    double m2cost = 0.0;
    int cntMaxTimeOverKappa = 0;

    double expectedCostMC = ReducedModel::evaluateMarkovChain(reducedModel);
    cout << "Expected Cost of Reduced Model (Markov Chain): "
         << expectedCostMC << endl;

    for (int i = 0; i < nsims; i++) {
        double planningTime = 0.0;
        // Initial planning time
        // We don't want the simulations to re-use the computed values
        if (!flag_is_registered("no-reset")) {
            for (mlcore::State* s : wrapperProblem->states())
                s->reset();
        }
        startTime = clock();
        solver->solve(wrapperProblem->initialState());
        endTime = clock();
        if (!flag_is_registered("anytime")) {
            // Initial time always counted
            planningTime += (double(endTime - startTime) / CLOCKS_PER_SEC);
            if (verbosity >= 1000)
                cout << "initial planning time: "
                     << (double(endTime - startTime) / CLOCKS_PER_SEC)
                     << " cost " << wrapperProblem->initialState()->cost()
                     << endl;
        }
        double maxReplanningTimeCurrent = 0.0;
        pair<double, double> costAndTime;

        if (flag_is_registered("anytime")) {
            costAndTime = reducedModel->trialAnytime(
                *solver, wrapperProblem, &maxReplanningTimeCurrent);
        } else {
            costAndTime = reducedModel->trial(
                *solver, wrapperProblem, &maxReplanningTimeCurrent);
        }

        double delta = costAndTime.first - expectedCost;
        expectedCost += delta / (i + 1);
        double delta2 = costAndTime.first - expectedCost;
        m2cost += delta * delta2;
        maxReplanningTime = max(maxReplanningTime, maxReplanningTimeCurrent);
        planningTime += costAndTime.second;
        expectedTime += planningTime;
        if (maxReplanningTimeCurrent > kappa)
            cntMaxTimeOverKappa++;

        if (verbosity >= 100)
            cout << "trial ended: cost " << costAndTime.first << " " <<
                ", time " << planningTime << endl;
    }
    cout << "expected cost " << expectedCost << endl;
    cout << "var cost " << m2cost / (nsims - 1) << endl;
    cout << "expected planning time " << expectedTime / nsims << endl;
    cout << "max re-planning time " << maxReplanningTime << endl;
    cout << "cnt max re-planning time over kappa " <<
        cntMaxTimeOverKappa << endl;

    // Releasing memory
    for (auto reduction : reductions)
        delete reduction;
    reducedModel->cleanup();
    delete reducedModel;
    wrapperProblem->cleanup();
    delete wrapperProblem;
    delete problem;
    delete solver;
    return 0;
}
