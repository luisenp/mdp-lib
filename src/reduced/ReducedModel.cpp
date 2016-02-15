#include <list>
#include <vector>

#include "../../include/solvers/LAOStarSolver.h"

#include "../../include/reduced/ReducedHeuristicWrapper.h"
#include "../../include/reduced/ReducedModel.h"

namespace mlreduced
{

std::list<mlcore::Successor>
ReducedModel::transition(mlcore::State* s, mlcore::Action *a)
{
    ReducedState* rs = (ReducedState *) s;
    std::vector<bool> primaryIndicators =
        reducedTransition_->isPrimary(rs->originalState(), a);

    std::list<mlcore::Successor> successors;
    std::list<mlcore::Successor> originalSuccessors =
        originalProblem_->transition(rs->originalState(), a);
    double totalPrimaryProbability = 0.0;
    int i = 0;
    for (mlcore::Successor origSucc : originalSuccessors) {
        mlcore::State* next = nullptr;
        bool isPrimaryOutcome = primaryIndicators[i] || useFullTransition_;
        if (isPrimaryOutcome) {
            totalPrimaryProbability += origSucc.su_prob;
            next = addState(new ReducedState(origSucc.su_state,
                                             rs->exceptionCount(),
                                             this));
        } else if (rs->exceptionCount() <= k_) {
            if (rs->exceptionCount() < k_)
                next = addState(new ReducedState(origSucc.su_state,
                                                 rs->exceptionCount() + 1,
                                                 this));
            else if (useContPlanEvaluationTransition_)
                next = addState(new ReducedState(origSucc.su_state, 0, this));
        }
        if (useContPlanEvaluationTransition_ ||
                rs->exceptionCount() < k_ ||
                isPrimaryOutcome) {
            successors.push_back(mlcore::Successor(next, origSucc.su_prob));
        }
        i++;
    }

    if (rs->exceptionCount() == k_) {
        for (mlcore::Successor successor : successors) {
            successor.su_prob /= totalPrimaryProbability;
        }
    }
    return successors;
}


double ReducedModel::evaluateContinualPlan(ReducedModel* reducedModel,
                                            mlsolvers::Solver* solver)
{
    ReducedModel* markovChain =
        new ReducedModel(reducedModel->originalProblem_,
                         reducedModel->reducedTransition_,
                         reducedModel->k_);

    // First we generate all states that are reachable in the full model.
    markovChain->useFullTransition(true);
    markovChain->generateAll();

    // Then we create copies of all these states for j=1,...,k and add
    // them to the Markov Chain.
    std::list<mlcore::State *> statesFullModel(markovChain->states().begin(),
                                               markovChain->states().end());
    for (int j = 1; j <= reducedModel->k_; j++) {
        for (mlcore::State* s : statesFullModel) {
            ReducedState* rs = (ReducedState* ) s;
            markovChain->addState(
                new ReducedState(rs->originalState(), j, markovChain));
        }
    }
    // Finally, we make sure the MC uses the continual planning
    // transition function.
    markovChain->useContPlanEvaluationTransition(true);

    // Now we compute the expected cost of traversing this Markov Chain.
    double maxResidual = mdplib::dead_end_cost;
    while (maxResidual > 1.0e-3) {
        maxResidual = 0.0;
        for (mlcore::State* s : markovChain->states()) {
            if (markovChain->goal(s))
                continue;
            ReducedState* rs = (ReducedState *) s;
            mlcore::State* currentState =
                reducedModel->addState(new ReducedState(rs->originalState(),
                                                        rs->exceptionCount(),
                                                        reducedModel));
            if (currentState->bestAction() == nullptr)
                solver->solve(currentState);
            mlcore::Action *a = currentState->bestAction();
            double previousCost = s->cost();
            double currentCost = 0.0;
            for (mlcore::Successor successor : markovChain->transition(s, a)) {
                currentCost += successor.su_prob * successor.su_state->cost();
            }
            currentCost *= markovChain->gamma();
            currentCost += markovChain->cost(s, a);
            currentCost = std::min(currentCost, mdplib::dead_end_cost);
            double currentResidual = fabs(currentCost - previousCost);
            if (currentResidual > maxResidual) {
                maxResidual = currentResidual;
            }
            s->setCost(currentCost);
        }
    }
    return markovChain->initialState()->cost();
}


ReducedTransition*
ReducedModel::getBestReduction(
    mlcore::Problem *originalProblem,
    std::list<ReducedTransition *> reducedTransitions,
    int k,
    ReducedHeuristicWrapper* heuristic)
{
    double bestCost = mdplib::dead_end_cost + 1;
    ReducedTransition* bestReduction = nullptr;
    for (ReducedTransition* reducedTransition : reducedTransitions) {
        ReducedModel* reducedModel =
            new ReducedModel(originalProblem, reducedTransition, k);
        reducedModel->setHeuristic(heuristic);
        mlsolvers::LAOStarSolver solver(reducedModel, 1.0e-03);
        double expectedCostReduction =
            evaluateContinualPlan(reducedModel, &solver);
        dprint1(expectedCostReduction);
        if (expectedCostReduction < bestCost) {
            bestCost = expectedCostReduction;
            bestReduction = reducedTransition;
        }
    }
    return bestReduction;
}


} // namespace mlreduced
