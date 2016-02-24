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
EpicSolver::computeProbabilityTerminals(State* start,
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
                    problem_->goal(currentState) ||
                    currentState->deadEnd()) {
                (*nextProbabilities)[currentState] +=
                    (*previousProbabilities)[currentState];
                continue;
            }

            for (auto const & successor :
                    problem_->transition(currentState,
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


Action* EpicSolver::solve(State* start)
{
    State* currentState = start;
    list<State*> visitedStack;
    StateSet tmp;
    while (true) {
        if (problem_->goal(currentState))
            break;
        visitedStack.push_front(currentState);
        tmp.insert(currentState);

        bellmanUpdate(problem_, currentState);

        if (currentState->deadEnd())
            break;

        currentState =
            randomSuccessor(problem_, currentState, currentState->bestAction());
    }
    dprint2("different visited states ", tmp.size());
    dprint2("size of the visited stack", visitedStack.size());

    WrapperProblem* wrapper = new WrapperProblem(problem_);
    VISolver viSolver(wrapper);
    StateSet reachableStates, tipStates, goalTargets;
    wrapper->overrideStates(&reachableStates);
    wrapper->overrideGoals(&goalTargets);
    while (!visitedStack.empty()) {
        currentState = visitedStack.front();
        visitedStack.pop_front();
        if (goalTargets.count(currentState) > 0)
            continue;
        bool containsGoal = false;
        reachableStates.clear();
        wrapper->setNewInitialState(currentState);
        int horizon = 0;
        while (!containsGoal) {
            containsGoal =
                getReachableStates(wrapper, reachableStates, tipStates, 1);
            horizon++;
        }
        goalTargets.insert(tipStates.begin(), tipStates.end());
        viSolver.solve();
        goalTargets.insert(reachableStates.begin(), reachableStates.end());
        wrapper->overrideGoals(nullptr);
        wrapper->overrideGoals(&goalTargets);
    }

    wrapper->cleanup();
    delete wrapper;
}

} //namespace mlsolvers
