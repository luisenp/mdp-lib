#include <cassert>
#include <ctime>
#include <string>

#include "../../include/Problem.h"
#include "../../include/domains/racetrack/RacetrackProblem.h"
#include "../../include/domains/racetrack/RTrackDetHeuristic.h"

#include "../../include/reduced/LeastLikelyOutcomeReduction.h"
#include "../../include/reduced/RacetrackObviousReduction.h"
#include "../../include/reduced/ReducedHeuristicWrapper.h"
#include "../../include/reduced/ReducedModel.h"
#include "../../include/reduced/ReducedTransition.h"

#include "../../include/solvers/Solver.h"
#include "../../include/solvers/LAOStarSolver.h"

#include "../../include/util/flags.h"
#include "../../include/util/general.h"

using namespace std;
using namespace mdplib;
using namespace mlcore;
using namespace mlreduced;
using namespace mlsolvers;

int main(int argc, char* args[])
{
    register_flags(argc, args);

    assert(flag_is_registered("track"));
    string trackName = flag_value("track");

    int verbosity = 0;
    if (flag_is_registered_with_value("v"))
        verbosity = stoi(flag_value("v"));

    int mds = -1;
    if (flag_is_registered_with_value("mds"))
        mds = stoi(flag_value("mds"));

    Problem* problem = new RacetrackProblem(trackName.c_str());
    ((RacetrackProblem*) problem)->pError(0.05);
    ((RacetrackProblem*) problem)->pSlip(0.10);
    ((RacetrackProblem*) problem)->mds(mds);

    problem->generateAll();

    if (verbosity > 100)
        cout << "Generated " << problem->states().size() << " states." << endl;

    mdplib_debug = false;
    Heuristic* heuristic = new RTrackDetHeuristic(trackName.c_str());
    Heuristic* reducedHeuristic = new ReducedHeuristicWrapper(heuristic);

    int k = 0;
    if (flag_is_registered_with_value("k"))
        k = stoi(flag_value("k"));
    ReducedTransition* obviousReduction =
        new RacetrackObviousReduction((RacetrackProblem *) problem);
    Problem* reducedModel = new ReducedModel(problem, obviousReduction, k);
    reducedModel->setHeuristic(reducedHeuristic);

    if (flag_is_registered("use_full"))
        ((ReducedModel *)reducedModel)->useFullTransition(true);

    mdplib_debug = true;
    reducedModel->generateAll();

    if (verbosity > 100)
        cout << "Generated " << reducedModel->states().size() <<
            " reduced states." << endl;

    double totalPlanningTime = 0.0;
    clock_t startTime = clock();
    LAOStarSolver lao(reducedModel, 1.0e-3);
    lao.solve(reducedModel->initialState());
    clock_t endTime = clock();
    totalPlanningTime += (double(endTime - startTime) / CLOCKS_PER_SEC);

    double expCostCP =
        ReducedModel::evaluateContinualPlan((ReducedModel *) reducedModel, &lao);
    cout << expCostCP << endl;

    ReducedTransition* lloReduction =
        new LeastLikelyOutcomeReduction((RacetrackProblem *) problem);
    std::list<ReducedTransition *> reductions;
    reductions.push_back(obviousReduction);
    reductions.push_back(lloReduction);
    ReducedTransition* bestReduction =
        ReducedModel::getBestReduction(problem,
                                       reductions,
                                       k,
                                       (ReducedHeuristicWrapper *) reducedHeuristic);
    assert(bestReduction == obviousReduction);

    double cost = 0.0;
    State* currentState = reducedModel->initialState();
    ReducedState* rs = (ReducedState *) currentState;
    while (!reducedModel->goal(currentState)) {
        if (verbosity > 100)
            cout << currentState << "  " << currentState->bestAction() << endl;

        int exceptionCount = rs->exceptionCount();

        // In the simulation we want to use the full transition function.
        // To do this we set the exception counter of the current state to -1
        // so that it's guaranteed to be lower than the exception bound k
        // and the reduced model is forced to use the full transition.
        // We don't use reducedModel->useFullTransition(true) because we still
        // want to know if the outcome was an exception or not.
        ReducedState* auxState =
            new ReducedState(rs->originalState(), -1, reducedModel);
        mlcore::Action* bestAction = currentState->bestAction();
        cost += reducedModel->cost(rs, bestAction);
        currentState = randomSuccessor(reducedModel, auxState, bestAction);

        // Adjusting to the correct the exception count.
        rs = (ReducedState *) currentState;
        if (rs->exceptionCount() == -1)
            rs->exceptionCount(exceptionCount);
        else
            rs->exceptionCount(exceptionCount + 1);
        currentState = reducedModel->getState(currentState);

        if (currentState == nullptr) {
            assert(k == 0); // Only determinization should reach here.
            if (verbosity > 100)
                cout << "No plan for this state. Re-planning." << endl;
            rs->exceptionCount(0);
            currentState = reducedModel->addState(rs);
            startTime = clock();
            lao.solve(currentState);
            endTime = clock();
            totalPlanningTime += (double(endTime - startTime) / CLOCKS_PER_SEC);
        } else if (rs->exceptionCount() == k) {
            if (verbosity > 100)
                cout << "Pro-active re-planning." << endl;
            rs->exceptionCount(0);
            currentState = reducedModel->addState(rs);
            startTime = clock();
            lao.solve(currentState);
            endTime = clock();
            totalPlanningTime += (double(endTime - startTime) / CLOCKS_PER_SEC);
        }
    }
    if (verbosity > 100) {
        cout << "Total cost " << cost << endl;
        cout << "Total planning time " << totalPlanningTime << endl;
    } else
        cout << cost << " " << totalPlanningTime << endl;

    return 0;
}
