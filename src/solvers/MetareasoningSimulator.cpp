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

}
