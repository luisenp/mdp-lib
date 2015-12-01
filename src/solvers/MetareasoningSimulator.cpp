#include <cassert>

#include "../../include/mdplib.h"
#include "../../include/solvers/solver.h"

#include "../../include/solvers/MetareasoningSimulator.h"

namespace mlsolvers
{

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
            std::cout << s << " " << stateValues_.back()[s] << " ";
        }
        std::cout << "residual " << maxResidual << " " << std::endl;

        // Storing the cost of the current policy
        mlcore::StateDoubleMap currentPolicyCost;
        computeExpectedCostCurrentPolicy(currentPolicyCost);
        policyCosts_.push_back(currentPolicyCost);
        for (mlcore::State* s : problem_->states()) {
            std::cerr << s << " " << policyCosts_.back()[s] << " ";
        }
        std::cout << " " << std::endl << " " << std::endl;

        dsleep(500);

        if (maxResidual < tolerance_)
            break;
    }
}

void MetareasoningSimulator::simulate()
{
    mlcore::State* currentState = problem_->initialState();
    int time = 0;
    double cost = 0;
    while (true) {
        if (problem_->goal(currentState))
            break;
        mlcore::Action* action = nullptr;
        switch(rule_) {
            case META_ASSUMPTION_1:
                action = getActionMetaAssumption1(currentState, time);
                break;
            case META_ASSUMPTION_2:
                break;
            case NO_META:
                break;
        }
        if (action == nullptr)
            time += numPlanningStepsPerNOP_;
        else
            time += numPlanningStepsPerAction_;
    }
}

mlcore::Action*
MetareasoningSimulator::getActionMetaAssumption1(mlcore::State* s, int t)
{
    // The time after executing an action.
    // Capped at policyCosts_.size() - 1 because at that point the policy is
    // optimal anyway
    int tNextAction = std::min(t + numPlanningStepsPerAction_,
                         (int) policyCosts_.size() - 1);

    // Computing the QValue of the action chosen by the current plan, assuming
    // the plan after one action execution will remain unchanged.
    double qValueCurrentPlan = mdplib::dead_end_cost + 1;
    mlcore::Action* bestActionCurrentPlan;
    for (mlcore::Action* a : problem_->actions()) {
        if (!problem_->applicable(s, a))
            continue;
        double qValueAction = 0.0;
        for (mlcore::Successor su : problem_->transition(s, a)) {
            qValueAction += su.su_prob * policyCosts_[tNextAction][su.su_state];
        }
        qValueAction =
            (qValueAction * problem_->gamma()) + problem_->cost(s, a);
        if (qValueAction < qValueCurrentPlan) {
            qValueCurrentPlan = qValueAction;
            bestActionCurrentPlan = a;
        }
    }
    assert(bestActionCurrentPlan != nullptr);

    // The time after executing NOP.
    int tNextNOP = std::min(t + numPlanningStepsPerNOP_,
                         (int) policyCosts_.size() - 1);
    double qValueNOP =
        costNOP_ + problem_->gamma() * policyCosts_[tNextNOP][s];

    if (qValueNOP < qValueCurrentPlan)
        return nullptr;
    return bestActionCurrentPlan;
}

}
