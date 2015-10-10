#include "../../include/solvers/MetareasoningSolver.h"

namespace mlsolvers
{
/*********************************************************/
/** ******************** README!!! ******************** **/
/** Right now this is just a simulation of the concurrent
  * metareasoning approach. The planning and execution
  * occur separately, under the assumption the during the
  * time it takes to execution one action there are exactly
  * X LAO* searches (for some fixed value X).
/*********************************************************/

mlcore::Action* MetareasoningSolver::solve(mlcore::State* s)
{
    // Estimating the best action taking into account the current state of the planner.
    mlcore::Action* bestAction = nullptr;
    double bestQValue = mdplib::dead_end_cost + 1;
    for (mlcore::Action* a : problem_->actions()) {
        double estimatedQValueAction = estimateQValueAction(s, a);
        if (estimatedQValueAction < bestQValue) {
            bestAction = a;
            bestQValue = estimatedQValueAction;
        }
    }
    // Simulate the planner running during the next action.
    int numSearches = 3;
    for (int i = 0; i < numSearches; i++) {
        visited.clear();
        expand(s);
    }
    return bestAction;
}

int MetareasoningSolver::expand(mlcore::State* s)
{
    if (!visited.insert(s).second)  // state was already visited
        return 0;

    if (s->deadEnd() || problem_->goal(s))
        return 0;

    int cnt = 0;
    if (s->bestAction() == nullptr) {   // this means state has not been expanded
        bellmanUpdate(problem_, s);
        return 1;
    } else {
        mlcore::Action* a = s->bestAction();
        for (mlcore::Successor sccr : problem_->transition(s, a))
            cnt += expand(sccr.su_state);
    }
    bellmanUpdate(problem_, s);
    return cnt;
}

double MetareasoningSolver::estimateQValueAction(mlcore::State* s, mlcore::Action* a)
{
    int numSamples = 100;
    int horizon = 50;
    double QValueEstimate = 0.0;
    for (int i = 0; i < numSamples; i++) {
        // Add cost of taking this action first, and sample a successor.
        double trialCost = problem_->cost(s, a);
        mlcore::State* currentState = randomSuccessor(problem_, s, a);
        int steps = 1;
        while (!problem_->goal(currentState) && steps < horizon) {
            mlcore::Action *predictedAction = predictNextAction(problem_, s);
            trialCost += problem_->cost(s, predictedAction);
            currentState = randomSuccessor(problem_, s, predictedAction);
        }
        QValueEstimate += trialCost;
    }
    return QValueEstimate /= numSamples;
}

} // mlsolvers
