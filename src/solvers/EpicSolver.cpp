#include <list>
#include <utility>

#include "../../include/domains/WrapperProblem.h"

#include "../../include/solvers/EpicSolver.h"
#include "../../include/solvers/VISolver.h"

using namespace std;
using namespace mlcore;

namespace mlsolvers
{

StateDoubleMap
EpicSolver::computeProbabilityTerminals(Problem* problem,
                                        State* start,
                                        StateSet& terminals)
{
    StateSet visited;
    StateDoubleMap sdMap1, sdMap2;
    StateDoubleMap* previousProbabilities = &sdMap1;
    StateDoubleMap* nextProbabilities = &sdMap2;
    (*previousProbabilities)[start] = 1.0;
    double maxDiff = 1.0;
    while (true) {
        maxDiff = 0.0;
        list<State*> queue;
        queue.push_front(start);
        while (!queue.empty()) {
            State* currentState = queue.back();
            queue.pop_back();
            if (!visited.insert(currentState).second)
                continue;
            if (terminals.count(currentState) > 0 ||
                    problem->goal(currentState) ||
                    currentState->deadEnd()) {
                (*nextProbabilities)[currentState] +=
                    (*previousProbabilities)[currentState];
                continue;
            }
            for (auto const & successor :
                    problem->transition(currentState,
                                         currentState->bestAction())) {
                (*nextProbabilities)[successor.su_state] +=
                    (*previousProbabilities)[currentState] * successor.su_prob;
                queue.push_front(successor.su_state);
            }
        }

        for (auto s : visited) {
            maxDiff = max(maxDiff,
                          fabs((*nextProbabilities)[s] -
                              (*previousProbabilities)[s]));

        }
        if (maxDiff < mdplib::epsilon)
            break;
        swap(previousProbabilities, nextProbabilities);
        nextProbabilities->clear();
        visited.clear();
    }
    return *nextProbabilities;
}


void EpicSolver::trial(State* state)
{
    State* currentState = state;
    list<State*> visitedStack;
    while (true) {
        if (problem_->goal(currentState))
            break;
        visitedStack.push_front(currentState);
        bellmanUpdate(problem_, currentState);
        if (currentState->deadEnd())
            break;
        currentState =
            randomSuccessor(problem_, currentState, currentState->bestAction());
    }

    WrapperProblem* wrapper = new WrapperProblem(problem_);
    StateSet reachableStates, goalTargets;
    wrapper->overrideStates(&reachableStates);
    wrapper->overrideGoals(&goalTargets);
    while (!visitedStack.empty()) {
        currentState = visitedStack.front();
        visitedStack.pop_front();
        solveDepthLimited(currentState, wrapper);
    }
    wrapper->cleanup();
    delete wrapper;
}


// Below we use the term "solved" loosely. The values computed are only an
// admissible heuristic, not necessarily the optimal values.
void EpicSolver::solveDepthLimited(State* state, WrapperProblem* wrapper)
{
    VISolver viSolver(wrapper);
    StateSet tipStates;

    if (wrapper->overrideGoals()->count(state) > 0) {
        return; // This state has already been solved.
    }

    // We first find all states reachable up to a predetermined horizon and the
    // corresponding tip states (states that were solved before or that are at
    // the horizon). Note that states that have already been solved have also
    // been added to wrapper->overrideGoals(), so they will be considered
    // tip states by getReachableStates().
    wrapper->setNewInitialState(state);
    wrapper->overrideStates()->clear();
    assert(getReachableStates(wrapper,
                              *wrapper->overrideStates(),
                              tipStates,
                              horizon_));


    // We now find the best way to reach one of the previously solved
    // states. WrapperProblem is implemented so that states in
    // wrapper->overrideGoals_transition to an absorbing state
    // with a cost equal to their estimated cost so far.
    wrapper->overrideGoals()->insert(tipStates.begin(), tipStates.end());
    viSolver.solve();

    // And finally we add to the set of goals all states that were
    // solved during this iteration.
    wrapper->overrideGoals()->insert(wrapper->overrideStates()->begin(),
                                     wrapper->overrideStates()->end());
}


Action* EpicSolver::solve(State* start)
{
    trial(start);
    return start->bestAction();
}

} //namespace mlsolvers
