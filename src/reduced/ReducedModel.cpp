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

    // If the transition was already deterministic, then no reduction is
    // possible, just use the same transition w/o increasing the counter.
    if (originalSuccessors.size() == 1) {
        Successor const & origSucc = originalSuccessors.back();
        State* next = addState(
            new ReducedState(origSucc.su_state, rs->exceptionCount(), this));
        successors.push_back(Successor(next, 1.0));
        return successors;
    }

    double totalProbability = 0.0;
    int i = 0;
    for (Successor const & origSucc : originalSuccessors) {
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

    bool safe = mlsolvers::testDeadEnds(reducedModel);
    if (!safe) {
        return mdplib::dead_end_cost;
    }

//                                                                                mlcore::StateDoubleMap heur;
//                                                                                for (mlcore::State* sss : reducedModel->states()) {
//                                                                                    heur[sss] = sss->cost();
//                                                                                }

    // Computing an universal plan for all of these states in the reduced model.
    mlsolvers::VISolver solver(reducedModel, 1000000, 1.0e-3);
    solver.solve();
//                                                                                for (mlcore::State* sss : reducedModel->states()) {
////                                                                                    dprint(sss, heur.at(sss), sss->cost());
//                                                                                    if (heur[sss] > sss->cost()) {
//                                                                                        dprint("error", sss, heur[sss], sss->cost());
//                                                                                    }
//                                                                                }

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



std::pair<double, double>
ReducedModel::trial(mlsolvers::Solver & solver,
                    WrapperProblem* wrapperProblem,
                    double* maxPlanningTime) {
    assert(wrapperProblem->problem() == this);

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
//                                                                                generateAll();
//                                                                                for (mlcore::State* sss : states_) {
//                                                                                    dprint(sss);
//                                                                                    for (mlcore::Action* aaa : actions_) {
//                                                                                        if (!applicable(sss, aaa))
//                                                                                            continue;
//                                                                                        dprint("  ", aaa);
//                                                                                        for (auto& susu : transition(sss, aaa)) {
//                                                                                            dprint("    ", susu.su_state, susu.su_prob);
//                                                                                        }
//                                                                                    }
//                                                                                }


//                                                                                mlcore::StateSet bpsg;
//                                                                                mlsolvers::getBestPartialSolutionGraph(this, currentState, bpsg);
//                                                                                for (mlcore::State* sss : bpsg) {
//                                                                                    if (goal(sss)) {
//                                                                                        dprint(sss);
//                                                                                        continue;
//                                                                                    }
//                                                                                    dprint("parent", sss);
//                                                                                    for (mlcore::Action* aaa : actions_) {
//                                                                                        if (!applicable(sss, aaa))
//                                                                                            continue;
//                                                                                        std::string prefix = aaa == sss->bestAction() ? "**" : "--";
//                                                                                        dprint(prefix, aaa);
//                                                                                        double qvalue = this->cost(sss, aaa);
//                                                                                        for (auto& susu : transition(sss, aaa)) {
//                                                                                            dprint("    ", susu.su_state, susu.su_prob, susu.su_state->cost());
//                                                                                            qvalue += susu.su_prob * susu.su_state->cost();
//                                                                                        }
//                                                                                        dprint("  qvalue", qvalue);
//                                                                                    }
//                                                                                }

    // This state will be used to simulate the full transition function by
    // making it a copy of the current state and adjusting the exception counter
    // accordingly.
    ReducedState* auxState = new ReducedState(*currentState);
    bool resetExceptionCounter = false;


//                                                                                dprint("*****************************************");
//                                                                                dprint("initial state", currentState);
    while (true) {
        Action* bestAction = currentState->bestAction();
//                                                                                for (auto& sss : transition(currentState, bestAction)) {
//                                                                                    dprint("  ", sss.su_state, sss.su_prob);
//                                                                                }
        cost += this->cost(currentState, bestAction);
        int exceptionCount = currentState->exceptionCount();

        if (cost >= mdplib::dead_end_cost)
            break;

        // Simulating the action execution using the full model.
        // Since we want to use the full transition function for this,
        // we set the exception counter of the current state to k + 1
        // so that it's guaranteed to be higher than the
        // exception bound k, thus forcing the reduced model to use the full
        // transition. We don't use reducedModel->useFullTransition(true)
        // because we still want to know if the outcome was an exception or not.
        // TODO: Write a method in ReducedModel that does this because this
        // approach will make reducedModel store the copies with j=-1.
        auxState->originalState(currentState->originalState());
        auxState->exceptionCount(this->k_ + 1);
        ReducedState* nextState = static_cast<ReducedState*>(
            mlsolvers::randomSuccessor(this, auxState, bestAction));
        auxState->originalState(nextState->originalState());
        auxState->exceptionCount(nextState->exceptionCount());

        // Adjusting the result to the current exception count.
        if (resetExceptionCounter) {
//                                                                                dprint("reset counter");
            // We reset the exception counter after pro-active re-planning.
            auxState->exceptionCount(this->k_);
            resetExceptionCounter = false;
        } else {
//                                                                                dprint("dont reset counter", auxState->exceptionCount());
            // no exception happened.
            if (auxState->exceptionCount() == this->k_ + 1)
                auxState->exceptionCount(exceptionCount);
            else
                auxState->exceptionCount(exceptionCount - 1);
        }

        nextState =
            static_cast<ReducedState*>(this->getState(auxState));
//                                                                                dprint("next state", nextState);

        if ((nextState != nullptr && nextState->deadEnd()) ||
                cost >= mdplib::dead_end_cost) {
            cost = mdplib::dead_end_cost;
            break;
        }

        if (nextState != nullptr && this->goal(nextState)) {
            break;
        }

        // Re-planning
        // Checking if the state has already been considered during planning.
        if (nextState == nullptr || nextState->bestAction() == nullptr) {
            // State wasn't considered before.
            assert(this->k_ == 0);  // Only determinization should reach here.
            auxState->exceptionCount(0);
            nextState = static_cast<ReducedState*>(
                this->addState(new ReducedState(*auxState)));
            double planningTime =
                triggerReplan(solver, nextState, false, wrapperProblem);
            totalPlanningTime += planningTime;
            if (maxPlanningTime)
                *maxPlanningTime = std::max(*maxPlanningTime, planningTime);

            assert(nextState != nullptr);
        } else if (!this->useFullTransition_) {
            if (this->k_ != 0 && nextState->exceptionCount() == 0) {
//                                                                                dprint("  replanning");
                double planningTime =
                    triggerReplan(solver, nextState, true, wrapperProblem);
                if (planningTime > kappa_) {
                    // Assumes agent idles while waiting for planning to end
                    totalPlanningTime += (planningTime - kappa_);
                }
                if (maxPlanningTime) {
                    *maxPlanningTime = std::max(*maxPlanningTime, planningTime);
                }
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
                                   WrapperProblem* wrapperProblem) {
    if (this->goal(nextState))
        return 0.0;

    if (proactive) {
        Action* bestAction = nextState->bestAction();
        // This action can't be null because we are planning pro-actively.
        assert(bestAction != nullptr);

        // We plan for all successors of the nextState under the full
        // model. The (k + 1) is used to get the full model transition
        // (see comment above in the trial function).
        ReducedState tmp(nextState->originalState(), this->k_ + 1, this);
        std::list<Successor> successorsFullModel =
            this->transition(&tmp, bestAction);

        // Adding the successors of nextState (under full transition) as
        // successors of the dummy initial state
        std::list<Successor> dummySuccessors;
        for (Successor const & sccr : successorsFullModel) {
            ReducedState* reducedSccrState =
                static_cast<ReducedState*>(
                    this->addState(new ReducedState(
                        static_cast<ReducedState*>(sccr.su_state)->
                            originalState(),
                        this->k_,
                        this)));
            dummySuccessors.push_back(
                Successor(reducedSccrState, sccr.su_prob));
        }
        wrapperProblem->setDummyAction(bestAction);
        wrapperProblem->dummyState()->setSuccessors(dummySuccessors);
        clock_t startTime = clock();
        solver.solve(wrapperProblem->dummyState());
        clock_t endTime = clock();
        return (double(endTime - startTime) / CLOCKS_PER_SEC);
    } else {
        clock_t startTime = clock();
        solver.solve(nextState);
        clock_t endTime = clock();
        return (double(endTime - startTime) / CLOCKS_PER_SEC);

    }
}


std::pair<double, double>
ReducedModel::trialAnytime(mlsolvers::Solver & solver,
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
        if (this->k_ == 0 && !this->increase_k_) {
            // Using reactive planning when increase_k is false
            double planningTime = triggerReplanAnytime(solver,
                                                       currentState,
                                                       false,
                                                       wrapperProblem);
            totalPlanningTime += planningTime;
            if (maxPlanningTime) {
                *maxPlanningTime = std::max(*maxPlanningTime, planningTime);
            }
            action = currentState->bestAction();
        } else {
            int k_reduced = this->k_;
//                                                                                dprint(currentState);
            while (true) {
                ReducedState* auxState = static_cast<ReducedState*> (
                    this->getState(new ReducedState(
                        currentState->originalState(), k_reduced, this)));
                if (auxState == nullptr) {
                    // The state has never seen before with counter [k_reduced]
                    // Note that to reach counter [k_reduced], the state with
                    // counter = [k_reduced] - 1 has to be solved, so we can
                    // use the action previously stored (i.e., just break loop)
//                                                                                dprint("  will use ", action);
                    break;
                }
                if (auxState->checkBits(mdplib::SOLVED)) {
                    action = auxState->bestAction();
                    k_reduced++;
//                                                                                dprint("  solved", auxState, k_reduced, action);
                } else {
                    // State seen before but not yet solved, use the last
                    // stored action (for the last solved k), or the greedy
                    // action if no action has been stored yet
//                                                                                dprint("  not solved yet", auxState);
                    if (action == nullptr) {
                        action = mlsolvers::greedyAction(this, auxState);
//                                                                                dprint("  no action stored, use greedy");
                    }
//                                                                                else dprint("  using:", action);
                    break;
                }
            }
        }

        if (action == nullptr) {
            cost = mdplib::dead_end_cost;   // Current state is a dead-end
            break;
        }

        // Planning "in parallel" to action execution
        double planningTime =
            triggerReplanAnytime(solver, currentState, true, wrapperProblem);
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


double ReducedModel::triggerReplanAnytime(mlsolvers::Solver& solver,
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
        int k_reduced = this->k_;
        while (true) {
            dummySuccessors.clear();
            bool allSolved = true;
            for (Successor const & sccr : successorsFullModel) {
                ReducedState* reducedSccrState = static_cast<ReducedState*>(
                    this->addState(
                        new ReducedState(sccr.su_state, k_reduced, this)));
                allSolved &= reducedSccrState->checkBits(mdplib::SOLVED);
                dummySuccessors.push_back(
                    Successor(reducedSccrState, sccr.su_prob));
            }
            if (allSolved && this->increase_k_) {
                k_reduced++;
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
