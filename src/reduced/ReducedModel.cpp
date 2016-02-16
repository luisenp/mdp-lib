#include <list>
#include <vector>

#include "../../include/solvers/VISolver.h"

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


double ReducedModel::evaluateContinualPlan(ReducedModel* reducedModel)
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
    for (int j = 0; j <= reducedModel->k_; j++) {
        for (mlcore::State* s : statesFullModel) {
            ReducedState* rs = (ReducedState* ) s;
            if (j > 0) {
                markovChain->addState(
                    new ReducedState(rs->originalState(), j, markovChain));
            }
            // We need to add a copy also in the reduced model, because some
            // of these states might be unreachable from its initial state.
            mlcore::State* aux = reducedModel->addState(
                new ReducedState(rs->originalState(), j, reducedModel));
        }
    }

    // Computing an universal plan for all of these states in the reduced model.
    mlsolvers::VISolver solver(reducedModel, 1000000, 1.0e-3);
    solver.solve();

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
            ReducedState* markovChainState = (ReducedState *) s;
            // currentState is the state that markovChainState represents in
            // the reduced model.
            mlcore::State* currentState =
                reducedModel->addState(
                    new ReducedState(markovChainState->originalState(),
                                     markovChainState->exceptionCount(),
                                     reducedModel));

            if (currentState->deadEnd()) {
                // state->deadEnd is set by the solver when there is no
                // applicable action in state.
                s->setCost(mdplib::dead_end_cost);
                continue;
            }

            assert(currentState->bestAction() != nullptr);
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
        double expectedCostReduction = evaluateContinualPlan(reducedModel);
        dprint1(expectedCostReduction);
        if (expectedCostReduction < bestCost) {
            bestCost = expectedCostReduction;
            bestReduction = reducedTransition;
        }
    }
    return bestReduction;
}


} // namespace mlreduced
