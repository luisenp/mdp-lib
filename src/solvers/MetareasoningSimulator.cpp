#include <cassert>

#include "../../include/mdplib.h"
#include "../../include/solvers/solver.h"

#include "../../include/solvers/MetareasoningSimulator.h"

namespace mlsolvers
{

MetareasoningSimulator::MetareasoningSimulator(mlcore::Problem* problem,
                        double tolerance,
                        int numPlanningStepsPerAction,
                        int numPlanningStepsPerNOP,
                        double costNOP,
                        bool tryAllActions,
                        ActionSelectionRule rule)
{
    problem_ = problem;
    tolerance_ = tolerance;
    numPlanningStepsPerAction_ = numPlanningStepsPerAction;
    numPlanningStepsPerNOP_ = numPlanningStepsPerNOP;
    costNOP_ = costNOP;
    rule_ = rule;
    tryAllActions_ = tryAllActions;
    precomputeAllExpectedPolicyCosts();
}


void MetareasoningSimulator::computeExpectedCostCurrentPolicy(
    mlcore::StateDoubleMap& expectedCosts_)
{
    // This method uses Iterative Policy Evaluation
    expectedCosts_.clear();
    int i = 0;
    while (true) {
        double maxResidual = 0.0;
                /******
        std::cout << i << std::endl;
                ******/
        for (mlcore::State* s : problem_->states()) {
            mlcore::Action* a = greedyAction(problem_, s);
            double newCost = 0.0;
            for (mlcore::Successor su : problem_->transition(s, a)) {
                newCost += su.su_prob * expectedCosts_[su.su_state];
            }
            newCost = (newCost * problem_->gamma()) + problem_->cost(s, a);
            double residual = fabs(newCost - expectedCosts_[s]);
            expectedCosts_[s] = newCost;
            if (residual > maxResidual)
                maxResidual = residual;
                /******
                std::cout << newCost << " ";
                i++;
                ****/
        }
                /******
        std::cout << std::endl;
                ******/
        if (maxResidual < tolerance_)
            break;
    }
}


void MetareasoningSimulator::precomputeAllExpectedPolicyCosts()
{
    mdplib_debug = true;
    while (true) {
        stateValues_.push_back(mlcore::StateDoubleMap());
        double maxResidual = 0.0;
        for (mlcore::State* s : problem_->states()) {
            double residual = bellmanUpdate(problem_, s);
            if (residual > maxResidual) {
                maxResidual = residual;
            }
        }

        // Storing the current value function
        for (mlcore::State* s : problem_->states()) {
            stateValues_.back()[s] = s->cost();
//            std::cout << s << " " << stateValues_.back()[s] << " ";
        }
//        std::cout << "residual " << maxResidual << " " << std::endl;

        // Storing the cost of the current policy
        mlcore::StateDoubleMap currentPolicyCost;
        computeExpectedCostCurrentPolicy(currentPolicyCost);
        policyCosts_.push_back(currentPolicyCost);

//        std::cout << policyCosts_.back()[problem_->initialState()] << std::endl;

        if (maxResidual < tolerance_)
            break;
    }
}


std::pair<double, double> MetareasoningSimulator::simulate()
{
    mlcore::State* currentState = problem_->initialState();
    int time = 0;
    double cost = 0.0;
    double totalNOPCost = 0.0;
    while (true) {
        if (problem_->goal(currentState))
            break;
        mlcore::Action* action = nullptr;
        switch(rule_) {
            case META_ASSUMPTION_1:
                action = getActionMetaAssumption1(currentState, time);
                break;
            case META_ASSUMPTION_2:
                action = getActionMetaAssumption2(currentState, time);
                break;
            case NO_META:
                action = getActionNoMetareasoning(currentState, time);
                break;
        }
        if (action == nullptr) {
//            dprint2(currentState, "NOP");
            time += numPlanningStepsPerNOP_;
            cost += costNOP_;
            totalNOPCost += costNOP_;
        }
        else {
//            dprint3(time, currentState, action);
            time += numPlanningStepsPerAction_;
            cost += problem_->cost(currentState, action);
            currentState = randomSuccessor(problem_, currentState, action);
        }
    }
    return std::make_pair(cost, totalNOPCost);
}


mlcore::Action*
MetareasoningSimulator::getActionMetaAssumption1(mlcore::State* s, int t)
{
    // The time after executing an action.
    // Maxed at policyCosts_.size() - 1 because at that point the policy is
    // optimal anyway.
    int tNextAction = std::min(t + numPlanningStepsPerAction_,
                         (int) policyCosts_.size() - 1);

    // Computing the true Q-Value of the action chosen by the current plan,
    // assuming the plan after one action execution will remain unchanged.
    mlcore::Action* actionCurrentPlan;
    double qValueAction = mdplib::dead_end_cost + 1;
    if (tryAllActions_) {
        for (mlcore::Action* a : problem_->actions()) {
            if (!problem_->applicable(s, a))
                continue;
            double qValueCurrentAction = 0.0;
            for (mlcore::Successor su :
                    problem_->transition(s, a)) {
                qValueCurrentAction +=
                    su.su_prob * policyCosts_[tNextAction][su.su_state];
            }
            qValueCurrentAction =
                (qValueCurrentAction * problem_->gamma()) +
                    problem_->cost(s, a);
            if (qValueCurrentAction < qValueAction) {
                qValueAction = qValueCurrentAction;
                actionCurrentPlan = a;
            }
        }
    } else {
        actionCurrentPlan = getActionNoMetareasoning(s, t);
        qValueAction = 0.0;
        for (mlcore::Successor su :
                problem_->transition(s, actionCurrentPlan)) {
            qValueAction += su.su_prob * policyCosts_[tNextAction][su.su_state];
        }
        qValueAction =
            (qValueAction * problem_->gamma()) +
                problem_->cost(s, actionCurrentPlan);
    }

    // The time after executing NOP.
    int tNextNOP = std::min(t + numPlanningStepsPerNOP_,
                         (int) policyCosts_.size() - 1);
    double qValueNOP = costNOP_ + problem_->gamma() * policyCosts_[tNextNOP][s];

    if (qValueNOP < qValueAction)
        return nullptr;
    return actionCurrentPlan;
}


mlcore::Action*
MetareasoningSimulator::getActionNoMetareasoning(mlcore::State* s, int t)
{
    double qValueCurrentPlan = mdplib::dead_end_cost + 1;
    mlcore::Action* actionCurrentPlan;
    // At t = policyCosts_.size() - 1 the policy is optimal.
    t = std::min(t, (int) policyCosts_.size() - 1);
    for (mlcore::Action* a : problem_->actions()) {
        if (!problem_->applicable(s, a))
            continue;
        double qValueAction = 0.0;
        for (mlcore::Successor su : problem_->transition(s, a)) {
            qValueAction += su.su_prob * stateValues_[t][su.su_state];
        }
        qValueAction =
            (qValueAction * problem_->gamma()) + problem_->cost(s, a);
        if (qValueAction < qValueCurrentPlan) {
            qValueCurrentPlan = qValueAction;
            actionCurrentPlan = a;
        }
    }
    assert(actionCurrentPlan != nullptr);
    return actionCurrentPlan;

}


mlcore::Action*
MetareasoningSimulator::getActionMetaAssumption2(mlcore::State* s, int t)
{
    // The time after executing an action.
    // Maxed at policyCosts_.size() - 1 because at that point the policy is
    // optimal anyway.
    int endTime = policyCosts_.size() - 1;
    int tNextAction = std::min(t + numPlanningStepsPerAction_, endTime);

    // Computing the true Q-Value of the action chosen by the current plan,
    // assuming the plan after one action execution will remain unchanged.
    mlcore::Action* actionCurrentPlan = getActionNoMetareasoning(s, t);
    double qValueAction = 0.0;
    for (mlcore::Successor su : problem_->transition(s, actionCurrentPlan)) {
        qValueAction += su.su_prob * stateValues_[endTime][su.su_state];
    }
    qValueAction =
        (qValueAction * problem_->gamma()) +
            problem_->cost(s, actionCurrentPlan);

    // The time after executing NOP.
    double qValueNOP = costNOP_ + problem_->gamma() * stateValues_[endTime][s];

    if (qValueNOP < qValueAction)
        return nullptr;
    return actionCurrentPlan;
}
}
