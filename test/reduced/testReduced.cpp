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

static int verbosity = 0;
static int k = 0;
Problem* problem = nullptr;
Heuristic* heuristic = nullptr;
Problem* reducedModel = nullptr;
Heuristic* reducedHeuristic = nullptr;


pair<double, double> simulate(Problem* reducedModel, Solver & solver)
{
    double cost = 0.0;
    double totalPlanningTime = 0.0;
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
            clock_t startTime = clock();
            solver.solve(currentState);
            clock_t endTime = clock();
            totalPlanningTime += (double(endTime - startTime) / CLOCKS_PER_SEC);
        } else if (rs->exceptionCount() == k) {
            if (verbosity > 100)
                cout << "Pro-active re-planning." << endl;
            rs->exceptionCount(0);
            currentState = reducedModel->addState(rs);
            clock_t startTime = clock();
            solver.solve(currentState);
            clock_t endTime = clock();
            totalPlanningTime += (double(endTime - startTime) / CLOCKS_PER_SEC);
        }
    }

    return make_pair(cost, totalPlanningTime);
}


void initRacetrack(string trackName, int mds)
{
    Problem* problem = new RacetrackProblem(trackName.c_str());
    ((RacetrackProblem*) problem)->pError(0.05);
    ((RacetrackProblem*) problem)->pSlip(0.10);
    ((RacetrackProblem*) problem)->mds(mds);
    heuristic = new RTrackDetHeuristic(trackName.c_str());
    problem->generateAll();
    if (verbosity > 100)
        cout << "Generated " << problem->states().size() << " states." << endl;

    ReducedTransition* obviousReduction =
        new RacetrackObviousReduction((RacetrackProblem *) problem);
    reducedModel = new ReducedModel(problem, obviousReduction, k);
    reducedHeuristic = new ReducedHeuristicWrapper(heuristic);
    reducedModel->setHeuristic(reducedHeuristic);
    reducedModel->generateAll();

    // Testing the code that tries different reductions of the racetrack.
    ReducedTransition* lloReduction =
        new LeastLikelyOutcomeReduction((RacetrackProblem *) problem);
    std::list<ReducedTransition *> reductions;
    reductions.push_back(obviousReduction);
    reductions.push_back(lloReduction);
    ReducedTransition* bestReduction =
        ReducedModel::getBestReduction(
            problem, reductions, k, (ReducedHeuristicWrapper *) reducedHeuristic);
    assert(bestReduction == obviousReduction);

}

int main(int argc, char* args[])
{
    register_flags(argc, args);

    // Reading flags.
    assert(flag_is_registered_with_value("domain"));
    string domainName = flag_value("domain");

    assert(flag_is_registered_with_value("problem"));

    if (flag_is_registered_with_value("v"))
        verbosity = stoi(flag_value("v"));


    if (flag_is_registered_with_value("k"))
        k = stoi(flag_value("k"));

    // Creating problem
    if (domainName == "racetrack") {
        int mds = -1;
        if (flag_is_registered_with_value("mds"))
            mds = stoi(flag_value("mds"));
        string trackName = flag_value("problem");
        initRacetrack(trackName, mds);
    }

    if (flag_is_registered("use_full"))
        ((ReducedModel *)reducedModel)->useFullTransition(true);

    // Solving reduced model using LAO*
    double totalPlanningTime = 0.0;
    clock_t startTime = clock();
    LAOStarSolver solver(reducedModel, 1.0e-3);
    solver.solve(reducedModel->initialState());
    clock_t endTime = clock();
    totalPlanningTime += (double(endTime - startTime) / CLOCKS_PER_SEC);

    // Running a trial of the continual planning approach using the reduced model.
    pair<double, double> costAndTime = simulate(reducedModel, solver);

    if (verbosity > 100) {
        cout << "Total cost " << costAndTime.first << endl;
        cout << "Total planning time " <<
            costAndTime.second + totalPlanningTime << endl;
    } else {
        cout << costAndTime.first << " "
            << costAndTime.second + totalPlanningTime << endl;
    }

    return 0;
}
