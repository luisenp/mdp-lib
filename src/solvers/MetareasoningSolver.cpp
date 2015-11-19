#include "../../include/solvers/MetareasoningSolver.h"

namespace mlsolvers
{
/*********************************************************/
/** ******************** README!!! ******************** **/
/** Right now this is just a simulation of the concurrent
  * meta-reasoning approach. The planning and execution
  * occur separately, under the assumption the during the
  * time it takes to execution one action there are exactly
  * X LAO* expansion steps (for some fixed value X).
/*********************************************************/

mlcore::Action* MetareasoningSolver::solve(mlcore::State* s)
{
    mdplib_debug = true;
    using_metareasoning = true;

    /********* This is here for debugging, it should be below!! **********/
    // Simulate the planner running during the next action
    int numSearches = 3;
    for (int i = 0; i < numSearches; i++) {
        visited_.clear();
        expand(s);
    }

    // This code is to test the accuracy of the action prediction estimator
//    std::vector<mlcore::Action *> predictedActions;
//    for (mlcore::State* tmpState : problem_->states()) {
//        predictedActions.push_back(predictNextAction(problem_, tmpState));
//    }

    mlcore::Action* bestAction = nullptr;
    if (useMetareasoning_) {
        double bestQValue = mdplib::dead_end_cost + 1;
        for (mlcore::Action* a : problem_->actions()) {
            if (!problem_->applicable(s, a))
                continue;
            double estimatedQValueAction = estimateQValueAction(s, a);
            if (estimatedQValueAction < bestQValue) {
                bestAction = a;
                bestQValue = estimatedQValueAction;
            }
        }
    } else {
        bestAction = greedyAction(problem_, s);
    }

    /******** The code up there should be here **********/

    // This code is to test the accuracy of the action prediction estimator
//    int i = 0;
//    double cntSuccess = 0;
//    double total = 0;
//    for (mlcore::State* tmpState : problem_->states()) {
//        if (greedyAction(problem_, tmpState) == predictedActions[i])
//            cntSuccess++;
//        total++;
//        i++;
//    dprint2("successes ", (cntSuccess/total));
//    }
    return bestAction;
}

int MetareasoningSolver::expand(mlcore::State* s)
{
    if (!visited_.insert(s).second) // state was already visited_
        return 0;

    if (s->deadEnd() || problem_->goal(s))
        return 0;

    int cnt = 0;
    if (s->bestAction() == nullptr) { // this means 's' has not been expanded
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

double
MetareasoningSolver::estimateQValueAction(mlcore::State* s, mlcore::Action* a)
{
    int numSamples = 1000;
    int horizon = 50;
    double QValueEstimate = 0.0;
    for (int i = 0; i < numSamples; i++) {
        // Add cost of taking this action first, and then sample a successor
        double trialCost = problem_->cost(s, a);
        mlcore::State* currentState = randomSuccessor(problem_, s, a);
        int steps = 1;
        while (!problem_->goal(currentState) && steps < horizon) {
            steps++;
            mlcore::Action *predictedAction =
                predictNextAction(problem_, currentState);
            trialCost += problem_->cost(currentState, predictedAction);
            currentState =
                randomSuccessor(problem_, currentState, predictedAction);
        }
        QValueEstimate += trialCost;
    }
    return QValueEstimate /= numSamples;
}

} // mlsolvers
