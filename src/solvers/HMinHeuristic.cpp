#include<list>

#include "../include/State.h"

#include "../../include/solvers/HMinHeuristic.h"
#include "../../include/solvers/Solver.h"


using namespace mlcore;
using namespace std;

namespace mlsolvers
{

void
 HMinHeuristic::hminUpdate(State* s)
{
    if (problem_->goal(s)) {
        costs_[s] = 0.0;
        return;
    }

    double bestQ = mdplib::dead_end_cost;
    bool hasAction = false;
    for (Action* a : problem_->actions()) {
        if (!problem_->applicable(s, a))
            continue;
        hasAction = true;
        double qAction = problem_->cost(s, a);
        double minCostSuccessor = mdplib::dead_end_cost;
        for (auto const & successor : problem_->transition(s, a)) {
            minCostSuccessor =
                std::min(minCostSuccessor, costs_[successor.su_state]);
        }
        qAction += minCostSuccessor;
        qAction = std::min(qAction, mdplib::dead_end_cost);
        if (qAction <= bestQ) {
            bestQ = qAction;
            bestActions_[s] = a;
        }
    }
    if (!hasAction) {
        s->markDeadEnd();
    }
    costs_[s] = bestQ;
}


HMinHeuristic::HMinHeuristic(Problem* problem, bool solveAll)
{
    problem_ = problem;
    solveAll_ = solveAll;
    if (solveAll_) {
        problem->generateAll();
        while (true) {
            double maxResidual = 0.0;
            for (State * s : problem->states()) {
                double prevCost = costs_[s];
                hminUpdate(s);
                maxResidual = std::max(maxResidual, fabs(costs_[s]- prevCost));
            }
            if (maxResidual < 1.0e-6)
                break;
        }
    }
}


double HMinHeuristic::cost(const State* s)
{
    if (problem_->goal(const_cast<State*>(s)))
        return 0.0;
    auto it = costs_.find(const_cast<State*> (s));
    if (it != costs_.end())
        return it->second;

    State* currentState = nullptr;
    while (true) {
        // Starting a LRTA* trial.
        currentState = const_cast<State*> (s);
        bool noActionChange = true;
        double maxResidual = 0.0;
        while (!problem_->goal(currentState)) {
            Action* bestAction = nullptr;
            if (bestActions_.count(currentState) > 0)
                bestAction = bestActions_[currentState];
            double prevCost = costs_[currentState];
            hminUpdate(currentState);
            if (currentState->deadEnd())
                break;
            if (bestAction != bestActions_.at(currentState))
                noActionChange == false;
            maxResidual = std::max(maxResidual,
                                   fabs(prevCost - costs_.at(currentState)));
            // Getting the successor of the best action.
            bestAction = bestActions_.at(currentState);
            double minCost = mdplib::dead_end_cost;
            State* nextState = nullptr;
            for (auto const & successor :
                    problem_->transition(currentState, bestAction)) {
                double successorCost = costs_.at(successor.su_state);
                if (successorCost < minCost) {
                    nextState = successor.su_state;
                    minCost = successorCost;
                }
            }
            currentState = nextState;
        }
        if (noActionChange && maxResidual < 1.0e-6)
            break;
    }

    it = costs_.find(const_cast<State*> (s));
    return it->second;
}

} // namespace mlsovers
