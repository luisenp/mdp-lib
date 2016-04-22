#include <ctime>
#include <list>
#include <vector>

#include "../../include/solvers/LAOStarSolver.h"
#include "../../include/solvers/VISolver.h"

#include "../../include/reduced/ReducedHeuristicWrapper.h"
#include "../../include/reduced/ReducedModel.h"


using namespace mlcore;


namespace mlreduced
{

std::list<Successor>
ReducedModel::transition(State* s, Action *a)
{
    ReducedState* rs = static_cast<ReducedState*>(s);
    std::vector<bool> primaryIndicators;
    if (!useFullTransition_)
        reducedTransition_->
            setPrimary(rs->originalState(), a, primaryIndicators);

    std::list<Successor> successors;
    std::list<Successor> originalSuccessors =
        originalProblem_->transition(rs->originalState(), a);
    double totalProbability = 0.0;
    int i = 0;
    for (Successor const & origSucc : originalSuccessors) {
        State* next = nullptr;
        bool isPrimaryOutcome = useFullTransition_ || primaryIndicators[i];
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
            successors.push_back(Successor(next, origSucc.su_prob));
            totalProbability += origSucc.su_prob;
        }
        i++;
    }
    for (Successor & successor : successors) {
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
    std::list<State*> statesFullModel(markovChain->states().begin(),
                                              markovChain->states().end());
    for (int j = 0; j <= reducedModel->k_; j++) {
        for (State* s : statesFullModel) {
            ReducedState* rs = static_cast<ReducedState*>(s);
            if (j > 0) {
                markovChain->addState(
                    new ReducedState(rs->originalState(), j, markovChain));
            }
            // We need to add a copy also in the reduced model, because some
            // of these states might be unreachable from its initial state.
            State* aux = reducedModel->addState(
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
        for (State* s : markovChain->states()) {
            if (markovChain->goal(s))
                continue;
            ReducedState* markovChainState = static_cast<ReducedState*>(s);
            // currentState is the state that markovChainState represents in
            // the reduced model.
            State* currentState =
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
            Action *a = currentState->bestAction();
            double previousCost = s->cost();
            double currentCost = 0.0;
            for (Successor successor : markovChain->transition(s, a)) {
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


ReducedTransition* ReducedModel::getBestReduction(
    Problem *originalProblem,
    std::list<ReducedTransition*> reducedTransitions,
    int k,
    ReducedHeuristicWrapper* heuristic)
{
    double bestCost = mdplib::dead_end_cost + 1;
    ReducedTransition* bestReduction = nullptr;
    for (ReducedTransition* reducedTransition : reducedTransitions) {
        ReducedModel reducedModel(originalProblem, reducedTransition, k);
        reducedModel.setHeuristic(heuristic);
        double expectedCostReduction = reducedModel.evaluateMonteCarlo(100);
        if (expectedCostReduction < bestCost) {
            bestCost = expectedCostReduction;
            bestReduction = reducedTransition;
        }
        for (State* s : reducedModel.states())
            s->reset();     // make sure stored values are cleared.
        reducedModel.cleanup();
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
                                                                                static int countGoals = 0;
                                                                                mdplib_debug = false;
    assert(wrapperProblem->problem() == this);

    double cost = 0.0;
    double totalPlanningTime = 0.0;
    ReducedState* currentState =
        static_cast<ReducedState*>(this->initialState());
    if (currentState->deadEnd())
        return std::make_pair(mdplib::dead_end_cost, 0.0);
    if (this->goal(currentState))
        return std::make_pair(0.0, 0.0);

    // This state will be used to simulate the full transition function by
    // making it a copy of the current state and adjusting the exception counter
    // accordingly.
    ReducedState* auxState = new ReducedState(*currentState);

    bool resetExceptionCounter = false;
    while (true) {
        Action* bestAction = currentState->bestAction();
        cost += this->cost(currentState, bestAction);
        int exceptionCount = currentState->exceptionCount();

        if (cost >= mdplib::dead_end_cost)
            break;

        // Simulating the action execution using the full model.
        // Since we want to use the full transition function for this,
        // we set the exception counter of the current state to -1
        // so that it's guaranteed to be lower than the
        // exception bound k, thus forcing the reduced model to use the full
        // transition. We don't use reducedModel->useFullTransition(true)
        // because we still want to know if the outcome was an exception or not.
        // TODO: Make a method in ReducedModel that does this because this
        // approach will make reducedModel store the copies with j=-1.
        auxState->originalState(currentState->originalState());
        auxState->exceptionCount(-1);
        ReducedState* nextState = static_cast<ReducedState*>(
            mlsolvers::randomSuccessor(this, auxState, bestAction));
        auxState->originalState(nextState->originalState());
        auxState->exceptionCount(nextState->exceptionCount());

        // Adjusting the result to the current exception count.
        if (resetExceptionCounter) {
            // We reset the exception counter after pro-active re-planning.
            auxState->exceptionCount(0);
            resetExceptionCounter = false;
        } else {
            if (auxState->exceptionCount() == -1)    // no exception happened.
                auxState->exceptionCount(exceptionCount);
            else
                auxState->exceptionCount(exceptionCount + 1);
        }
        nextState =
            static_cast<ReducedState*>(this->getState(auxState));

        if ((nextState != nullptr && nextState->deadEnd()) ||
                cost >= mdplib::dead_end_cost) {
            cost = mdplib::dead_end_cost;
            break;
        }
        if (nextState != nullptr && this->goal(nextState)) {
                                                                                countGoals++;
                                                                                mdplib_debug = true;
                                                                                dprint2("count goals", countGoals);
                                                                                mdplib_debug = false;
            break;
        }

        // Re-planning
        // Checking if the state has already been considered during planning.
                                                                                dprint1(nextState);
                                                                                if (nextState->bestAction() != nullptr)
                                                                                    dprint1(nextState->bestAction());

        if (nextState == nullptr || nextState->bestAction() == nullptr) {
            // State wasn't considered before.
            assert(this->k_ == 0);  // Only determinization should reach here.
            auxState->exceptionCount(0);
            nextState = static_cast<ReducedState*>(
                this->addState(new ReducedState(*auxState)));
            totalPlanningTime +=
                triggerReplan(solver, nextState, false, wrapperProblem);
            assert(nextState != nullptr);
        } else if (!this->useFullTransition_) {
            if (nextState->exceptionCount() == this->k_) {
                totalPlanningTime +=
                    triggerReplan(solver, nextState, true, wrapperProblem);
                resetExceptionCounter = true;
            }
        }
        currentState = nextState;
    }
    if (auxState != nullptr)
        delete auxState;
    return std::make_pair(cost, totalPlanningTime);
}


double ReducedModel::triggerReplan(mlsolvers::Solver& solver,
                                    ReducedState* nextState,
                                    bool proactive,
                                    WrapperProblem* wrapperProblem)
{
                                                                                dprint2("TRIGGER", nextState);
    if (this->goal(nextState))
        return 0.0;
    if (proactive) {
        Action* bestAction = nextState->bestAction();
        // This action can't be null because we are planning pro-actively.
        assert(bestAction != nullptr);

        // We plan for all successors of the nextState under the full
        // model. The -1 is used to get the full model transition (see comment
        // above in the trial function).
        ReducedState tmp(nextState->originalState(), -1, this);
        std::list<Successor> successorsFullModel =
            this->transition(&tmp, bestAction);
        std::list<Successor> dummySuccessors;
        for (Successor const & sccr : successorsFullModel) {
            ReducedState* reducedSccrState =
                static_cast<ReducedState*>(
                    this->addState(new ReducedState(
                        static_cast<ReducedState*>(sccr.su_state)->
                            originalState(),
                        0,
                        this)));
                                                                                dprint2("ADDING", reducedSccrState);
            dummySuccessors.push_back(
                Successor(reducedSccrState, sccr.su_prob));
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
