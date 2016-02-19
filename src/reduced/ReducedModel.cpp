#include <ctime>
#include <list>
#include <vector>

#include "../../include/solvers/LAOStarSolver.h"
#include "../../include/solvers/VISolver.h"

#include "../../include/reduced/ReducedHeuristicWrapper.h"
#include "../../include/reduced/ReducedModel.h"

namespace mlreduced
{

std::list<mlcore::Successor>
ReducedModel::transition(mlcore::State* s, mlcore::Action *a)
{
    ReducedState* rs = static_cast<ReducedState*>(s);
    std::vector<bool> primaryIndicators =
        reducedTransition_->isPrimary(rs->originalState(), a);

    std::list<mlcore::Successor> successors;
    std::list<mlcore::Successor> originalSuccessors =
        originalProblem_->transition(rs->originalState(), a);
    double totalProbability = 0.0;
    int i = 0;
    for (mlcore::Successor const & origSucc : originalSuccessors) {
        mlcore::State* next = nullptr;
        bool isPrimaryOutcome = primaryIndicators[i] || useFullTransition_;
        if (useContPlanEvaluationTransition_) {
            int add = isPrimaryOutcome && rs->exceptionCount() != k_ ? 0 : 1;
            next = addState(
                new ReducedState(origSucc.su_state,
                                 (rs->exceptionCount() + add) % (k_ + 1),
                                 this));
        } else {
            if (isPrimaryOutcome) {
                next = addState(new ReducedState(origSucc.su_state,
                                                 rs->exceptionCount(),
                                                 this));
            } else if (rs->exceptionCount() < k_) {
                next = addState(new ReducedState(origSucc.su_state,
                                                 rs->exceptionCount() + 1,
                                                 this));
            }
        }
        if (next != nullptr) {
            successors.push_back(mlcore::Successor(next, origSucc.su_prob));
            totalProbability += origSucc.su_prob;
        }
        i++;
    }
    for (mlcore::Successor & successor : successors) {
        successor.su_prob /= totalProbability;
    }
    return successors;
}


double ReducedModel::evaluateMarkovChain(ReducedModel* reducedModel)
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
    std::list<mlcore::State*> statesFullModel(markovChain->states().begin(),
                                              markovChain->states().end());
    for (int j = 0; j <= reducedModel->k_; j++) {
        for (mlcore::State* s : statesFullModel) {
            ReducedState* rs = static_cast<ReducedState*>(s);
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
                                                                                  dprint2("size", reducedModel->states().size());
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
            ReducedState* markovChainState = static_cast<ReducedState*>(s);
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
    std::list<ReducedTransition*> reducedTransitions,
    int k,
    ReducedHeuristicWrapper* heuristic)
{
    double bestCost = mdplib::dead_end_cost + 1;
    ReducedTransition* bestReduction = nullptr;
    for (ReducedTransition* reducedTransition : reducedTransitions) {
        ReducedModel* reducedModel =
            new ReducedModel(originalProblem, reducedTransition, k);
        reducedModel->setHeuristic(heuristic);
                                                                                  mdplib_debug = true;
//        double expectedCostReduction = evaluateContinualPlan(reducedModel);
        double expectedCostReduction = reducedModel->evaluateMonteCarlo(1000);
                                                                                  dprint2("reduction", expectedCostReduction);
                                                                                  mdplib_debug = false;
        if (expectedCostReduction < bestCost) {
            bestCost = expectedCostReduction;
            bestReduction = reducedTransition;
        }
    }
    return bestReduction;
}


double ReducedModel::evaluateMonteCarlo(int numTrials)
{
    WrapperProblem wrapper(this);
    mlsolvers::LAOStarSolver solver(static_cast<Problem*>(&wrapper));
    solver.solve(wrapper.initialState());
    double expectedCost = 0.0;
    for (int i = 0; i < numTrials; i++) {
        expectedCost += trial(solver, &wrapper).first;
    }
    wrapper.cleanup();
    return expectedCost /= numTrials;
}


std::pair<double, double> ReducedModel::trial(
    mlsolvers::Solver & solver, WrapperProblem* wrapperProblem)
{
    assert(wrapperProblem->problem() == this);
    double cost = 0.0;
    double totalPlanningTime = 0.0;
    ReducedState* currentState =
        static_cast<ReducedState*>(this->initialState());
    bool resetExceptionCounter = false;
    ReducedState auxState(*currentState);
    while (!this->goal(currentState)) {
        mlcore::Action* bestAction = currentState->bestAction();
        cost += this->cost(currentState, bestAction);
        int exceptionCount = currentState->exceptionCount();

        // Simulating the action execution using the full model.
        // Since we want to use the full transition function for this,
        // we set the exception counter of the current state to -1
        // so that it's guaranteed to be lower than the
        // exception bound k, thus forcing the reduced model to use the full
        // transition. We don't use reducedModel->useFullTransition(true)
        // because we still want to know if the outcome was an exception or not.
        // TODO: Make a method in ReducedModel that does this because this
        // approach will make reducedModel store the copies with j=-1.
//        auxState = *currentState;
        auxState.originalState(currentState->originalState());
        auxState.exceptionCount(-1);
        ReducedState* nextState = static_cast<ReducedState*>(
            mlsolvers::randomSuccessor(this, &auxState, bestAction));
//        auxState = *nextState;
        auxState.originalState(nextState->originalState());
        auxState.exceptionCount(nextState->exceptionCount());

        // Adjusting the result to the current exception count.
        if (resetExceptionCounter) {
            // We reset the exception counter after pro-active re-planning.
            auxState.exceptionCount(0);
            resetExceptionCounter = false;
        } else {
            if (auxState.exceptionCount() == -1)    // no exception happened.
                auxState.exceptionCount(exceptionCount);
            else
                auxState.exceptionCount(exceptionCount + 1);
        }
        nextState =
            static_cast<ReducedState*>(this->getState(&auxState));

        // Re-planning
        // Checking if the state has already been considered during planning.
        if (nextState == nullptr) {
            // State wasn't considered before.
            assert(this->k_ == 0);  // Only determinization should reach here.
            auxState.exceptionCount(0);
            nextState =
                static_cast<ReducedState*>(this->addState(&auxState));
            totalPlanningTime +=
                triggerReplan(solver, nextState, false, wrapperProblem);
            assert(nextState != nullptr);
        } else {
            if (nextState->exceptionCount() == this->k_) {
                totalPlanningTime +=
                    triggerReplan(solver, nextState, true, wrapperProblem);
                resetExceptionCounter = true;
            }
        }
        currentState = nextState;
    }

    return std::make_pair(cost, totalPlanningTime);
}


double ReducedModel::triggerReplan(mlsolvers::Solver& solver,
                                    ReducedState* nextState,
                                    bool proactive,
                                    WrapperProblem* wrapperProblem)
{
    if (this->goal(nextState))
        return 0.0;
    if (proactive) {
        mlcore::Action* bestAction = nextState->bestAction();
        // This action can't be null because we are planning pro-actively.
        assert(bestAction != nullptr);

        // We plan for all successors of the nextState under the full
        // model. The -1 is used to get the full model transition (see comment
        // above in the trial function).
        std::list<mlcore::Successor> successorsFullModel =
            this->transition(
                new ReducedState(
                    nextState->originalState(), -1, this),
                bestAction);
        std::list<mlcore::Successor> dummySuccessors;
        for (mlcore::Successor const & sccr : successorsFullModel) {
            ReducedState* reducedSccrState =
                static_cast<ReducedState*>(
                    this->addState(new ReducedState(
                        static_cast<ReducedState*>(sccr.su_state)->
                            originalState(),
                        0,
                        this)));
            dummySuccessors.push_back(
                mlcore::Successor(reducedSccrState, sccr.su_prob));
        }
        wrapperProblem->setDummyAction(bestAction);
        wrapperProblem->dummyState()->setSuccessors(dummySuccessors);
        solver.solve(wrapperProblem->dummyState());
        return 0.0;  // This happens in parallel to action execution.
    } else {
        clock_t startTime = clock();
        solver.solve(nextState);
        clock_t endTime = clock();
        return (double(endTime - startTime) / CLOCKS_PER_SEC);

    }
}


} // namespace mlreduced
