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
ReducedModel::transition(State* s, Action* a) {
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
//                                                                                dprint("--", s);
    for (Successor const & origSucc : originalSuccessors) {
//                                                                                dprint("------", origSucc.su_state, origSucc.su_prob);
        State* next = nullptr;
        bool isPrimaryOutcome =
            useFullTransition_ || primaryIndicators.empty() ||
            primaryIndicators.at(i);
        if (useContPlanEvaluationTransition_) {
            int add = isPrimaryOutcome && rs->exceptionCount() != 0 ? 0 : -1;
            int next_k = rs->exceptionCount();
            if (next_k == 0)
                next_k = this->k_;    // Simulates re-planning policy
            else
                next_k -= int(!isPrimaryOutcome);
            next = addState(new ReducedState(origSucc.su_state, next_k, this));
        } else {
            if (isPrimaryOutcome) {
                next = addState(new ReducedState(origSucc.su_state,
                                                 rs->exceptionCount(),
                                                 this));
            } else if (rs->exceptionCount() > 0) {
                next = addState(new ReducedState(origSucc.su_state,
                                                 rs->exceptionCount() - 1,
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


double ReducedModel::evaluateMarkovChain(ReducedModel* reducedModel) {
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
            Action* a = currentState->bestAction();
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
        Problem* originalProblem,
        std::list<ReducedTransition*> reducedTransitions,
        int k,
        ReducedHeuristicWrapper* heuristic) {
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


double ReducedModel::evaluateMonteCarlo(int numTrials) {
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


std::pair<double, double> ReducedModel::trial(mlsolvers::Solver & solver,
                                              WrapperProblem* wrapperProblem,
                                              double* maxPlanningTime) {
    int original_k = this->k_;
    double cost = 0.0;
    double totalPlanningTime = 0.0;
    if (maxPlanningTime)
        *maxPlanningTime = 0.0;
    ReducedState* currentState =
        static_cast<ReducedState*>(this->initialState());
    currentState->exceptionCount(this->k_);
    if (currentState->deadEnd())
        return std::make_pair(mdplib::dead_end_cost, 0.0);
    if (this->goal(currentState))
        return std::make_pair(0.0, 0.0);

    while (true) {

        Action* action = nullptr;
        if (this->k_ == 0) {
            // Using reactive planning
            double planningTime =
                triggerReplan(solver, currentState, false, wrapperProblem);
            totalPlanningTime += planningTime;
            if (maxPlanningTime) {
                *maxPlanningTime = std::max(*maxPlanningTime, planningTime);
            }
            action = currentState->bestAction();
        } else {
            action = mlsolvers::greedyAction(this, currentState);
        }

        if (action == nullptr) {
            cost = mdplib::dead_end_cost;   // Current state is a dead-end
            break;
        }

        // Planning "in parallel" to action execution
        double planningTime =
            triggerReplan(solver, currentState, true, wrapperProblem);
        if (maxPlanningTime) {
            *maxPlanningTime = std::max(*maxPlanningTime, planningTime);
        }

        cost += this->cost(currentState, action);

        if (cost >= mdplib::dead_end_cost)
            break;

        // Simulating the action execution using the full model
        mlcore::State* tmp =
            mlsolvers::randomSuccessor(this->originalProblem(),
                                       currentState->originalState(),
                                       action);
        ReducedState* auxState = new ReducedState(tmp, this->k_, this);
        ReducedState* nextState =
            static_cast<ReducedState*>(this->addState(auxState));

        // Checking for dead-ends
        if (nextState == nullptr) {
            cost = mdplib::dead_end_cost;
            break;
        }
        // Checking if the state is a goal
        if (nextState != nullptr && this->goal(nextState)) {
            break;
        }

        // Updating state for next step
        currentState = nextState;
    }
    this->k_ = original_k;;
    return std::make_pair(cost, totalPlanningTime);
}


double ReducedModel::triggerReplan(mlsolvers::Solver& solver,
                                   ReducedState* currentState,
                                   bool proactive,
                                   WrapperProblem* wrapperProblem) {
    if (this->goal(currentState))
        return 0.0;

    if (proactive) {
        Action* greedyAction = mlsolvers::greedyAction(this, currentState);

        // We plan for all successors of the currentState under the full
        // model.
        std::list<Successor> successorsFullModel =
            this->originalProblem()->transition(currentState->originalState(),
                                                greedyAction);

        // Adding the successors of currentState (under full transition) as
        // successors of the dummy initial state
        std::list<Successor> dummySuccessors;
        while (true) {
            dummySuccessors.clear();
            bool allSolved = true;
            for (Successor const & sccr : successorsFullModel) {
                ReducedState* reducedSccrState = static_cast<ReducedState*>(
                    this->addState(
                        new ReducedState(sccr.su_state, this->k_, this)));
                allSolved &= reducedSccrState->checkBits(mdplib::SOLVED);
                dummySuccessors.push_back(
                    Successor(reducedSccrState, sccr.su_prob));
            }
            if (allSolved && this->increase_k_) {
                this->k_++;
                                                                                dprint("all solved", this->k_);
            }
            else break;
        }
        wrapperProblem->setDummyAction(greedyAction);
        wrapperProblem->dummyState()->setSuccessors(dummySuccessors);
        clock_t startTime = clock();
        solver.solve(wrapperProblem->dummyState());
        // Clear all bits of dummy state to avoid incorrect labeling
        wrapperProblem->dummyState()->clearBits(~0);
        clock_t endTime = clock();
        return (double(endTime - startTime) / CLOCKS_PER_SEC);
    } else {
        clock_t startTime = clock();
        solver.solve(currentState);
        // Clear all bits of dummy state to avoid incorrect labeling
        wrapperProblem->dummyState()->clearBits(~0);
        clock_t endTime = clock();
        return (double(endTime - startTime) / CLOCKS_PER_SEC);
    }
}


bool ReducedModel::isException(
    mlcore::State* state, mlcore::State* successor, mlcore::Action* action) {
    bool isExcept = false;
    bool isNotExcept = false;
    ReducedState* rs = new ReducedState(state, this->k_, originalProblem_);
    for (auto const & sccr : this->transition(rs, action)) {
        ReducedState* reducedSuccessor =
            static_cast<ReducedState*> (sccr.su_state);
        if (reducedSuccessor->originalState() == successor) {
            if (reducedSuccessor->exceptionCount() == this->k_ - 1) {
                isExcept = true;
            } else {
                isNotExcept = true;
            }
        }
    }
    return isExcept;
}

} // namespace mlreduced
