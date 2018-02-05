#include <chrono>
#include <limits>

#include "../../include/MDPLib.h"
#include "../../include/State.h"

#include "../../include/domains/WrapperProblem.h"

#include "../../include/solvers/LRTDPSolver.h"
#include "../../include/solvers/Solver.h"
#include "../../include/solvers/SSiPPSolver.h"
#include "../../include/solvers/VISolver.h"


using namespace mlcore;
using namespace std;

namespace mlsolvers
{

Action* SSiPPSolver::solveOriginal(State* s0)
{
    beginTime_ = std::chrono::high_resolution_clock::now();
    if (maxTime_ > -1) {
        maxTrials_ = 10000000;
    }
    for (int i = 0; i < maxTrials_; i++) {
        mlcore::State* currentState = s0;
        while (!problem_->goal(currentState)) {
            // Creating the short-sighted SSP
            StateSet reachableStates, tipStates;
            if (useTrajProbabilities_) {
                getReachableStatesTrajectoryProbs(
                    problem_, currentState, reachableStates, tipStates, rho_);
            } else {
                reachableStates.insert(currentState);
                getReachableStates(problem_, reachableStates, tipStates, t_);
            }
            // Solving the short-sighted SSP
            WrapperProblem* wrapper = new WrapperProblem(problem_);
            wrapper->setNewInitialState(currentState);
            wrapper->overrideStates(&reachableStates);
            wrapper->overrideGoals(&tipStates);
            VISolver vi(wrapper, maxTrials_);
            auto endTime = std::chrono::high_resolution_clock::now();
            auto timeElapsed = std::chrono::duration_cast<
                std::chrono::milliseconds>(endTime - beginTime_).count();
            if (maxTime_ > -1) {
                                                                                dprint(std::max(0, maxTime_ - (int) timeElapsed));
                vi.maxPlanningTime(std::max(0, maxTime_ - (int) timeElapsed));
            }
            vi.solve();
            // Checking if it ran out of time
            if (maxTime_ > -1) {
                endTime = std::chrono::high_resolution_clock::now();
                timeElapsed = std::chrono::duration_cast<
                    std::chrono::milliseconds>(endTime - beginTime_).count();
                                                                                dprint(maxTime_, timeElapsed);
                if (timeElapsed > maxTime_) {
                    wrapper->cleanup();
                    delete wrapper;
                    break;
                }
            }
            // Execute the best action found for the current state.
            Action* action = currentState->bestAction();
            currentState = randomSuccessor(problem_, currentState, action);
            wrapper->cleanup();
            delete wrapper;
        }
        // Checking if it ran out of time
        if (maxTime_ > -1) {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto timeElapsed = std::chrono::duration_cast<
                std::chrono::milliseconds>(endTime - beginTime_).count();
            if (timeElapsed > maxTime_)
                break;
        }
    }
    return s0->bestAction();
}


Action* SSiPPSolver::solveLabeled(State* s0)
{
    beginTime_ = std::chrono::high_resolution_clock::now();
    while (!s0->checkBits(mdplib::SOLVED_SSiPP)) {
        State* currentState = s0;
        list<State*> visited;
        while (!currentState->checkBits(mdplib::SOLVED_SSiPP)) {
            visited.push_front(currentState);
            if (problem_->goal(currentState))
                break;
            // Constructing short-sighted SSP
            StateSet reachableStates, tipStates;
            if (useTrajProbabilities_) {
                getReachableStatesTrajectoryProbs(
                    problem_, currentState, reachableStates, tipStates, rho_);
            } else {
                reachableStates.insert(currentState);
                getReachableStates(problem_, reachableStates, tipStates, t_);
            }
            WrapperProblem wrapper(problem_);
            wrapper.overrideStates(&reachableStates);
            wrapper.overrideGoals(&tipStates);
            // Solving the short-sighted SSP
            optimalSolver(&wrapper, currentState);
            if (currentState->deadEnd())
                break;
            // Simulate best action
            currentState = randomSuccessor(problem_,
                                           currentState,
                                           greedyAction(problem_,
                                                        currentState));

            wrapper.cleanup();
            // Check if there is time remaining for planning
            if (maxTime_ > -1) {
                auto endTime = std::chrono::high_resolution_clock::now();
                auto timeElapsed = std::chrono::duration_cast<
                    std::chrono::milliseconds>(endTime - beginTime_).count();
                if (timeElapsed > maxTime_)
                    return s0->bestAction();
            }
        }
        while (!visited.empty()) {
            currentState = visited.front();
            visited.pop_front();
            if (!checkSolved(currentState))
                break;
        }
    }
    return s0->bestAction();
}


bool SSiPPSolver::checkSolved(State* s)
{
    std::list<State*> open, closed;

    State* tmp = s;
    if (!tmp->checkBits(mdplib::SOLVED_SSiPP)) {
        open.push_front(s);
        s->setBits(mdplib::CLOSED_SSiPP);
    }
    bool rv = true;
    while (!open.empty()) {
        tmp = open.front();
        open.pop_front();
        closed.push_front(tmp);
        Action* a = greedyAction(problem_, tmp);
        tmp->setBestAction(a);
        if (problem_->goal(tmp))
            continue;
        if (tmp->deadEnd()) {
            rv = false;
            continue;
        }
        if (residual(problem_, tmp) > epsilon_) {
            rv = false;
        }

        if (maxTime_ > -1) {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto timeElapsed = std::chrono::duration_cast<
                std::chrono::milliseconds>(endTime - beginTime_).count();
            if (timeElapsed > maxTime_) {
                rv = false;
                continue;
            }
        }

        for (Successor su : problem_->transition(tmp, a)) {
            State* next = su.su_state;
            if (!next->checkBits(mdplib::SOLVED_SSiPP) &&
                !next->checkBits(mdplib::CLOSED_SSiPP)) {
                open.push_front(next);
                next->setBits(mdplib::CLOSED_SSiPP);
            }
        }
    }
    if (rv) {
        for (State* sc : closed) {
            sc->setBits(mdplib::SOLVED_SSiPP);
        }
    } else {
        while (!closed.empty()) {
            tmp = closed.front();
            closed.pop_front();
            tmp->clearBits(mdplib::CLOSED_SSiPP);
            bellmanUpdate(problem_, tmp);
        }
    }
    return rv;
}


Action* SSiPPSolver::solve(State* s0)
{
    if (algorithm_ == SSiPPAlgo::Original)
        return solveOriginal(s0);
    if (algorithm_ == SSiPPAlgo::Labeled)
        return solveLabeled(s0);
}


void SSiPPSolver::optimalSolver(WrapperProblem* problem, State* s0)
{
    // This is a stack based implementation of LAO*.
    // We don't use the existing library implementation so that we can take
    // advantage of the SOLVED_SSiPP labels.
    StateSet visited;
    int countExpanded = 0;
    while (true) {
        do {
            visited.clear();
            countExpanded = 0;
            list<State*> stateStack;
            stateStack.push_back(s0);
            while (!stateStack.empty()) {
                State* s = stateStack.back();
                stateStack.pop_back();
                if (!visited.insert(s).second)  // state was already visited.
                    continue;
                if (s->deadEnd() ||
                        problem->goal(s) ||
                        s->checkBits(mdplib::SOLVED_SSiPP) ||
                        problem->overrideGoals()->count(s) > 0)
                    continue;
                int cnt = 0;
                if (s->bestAction() == nullptr) {
                    // state has never been expanded.
                    bellmanUpdate(problem, s);
                    countExpanded++;
                    continue;
                } else {
                    Action* a = s->bestAction();
                    for (Successor sccr : problem->transition(s, a))
                        stateStack.push_back(sccr.su_state);
                }
                if (!s->checkBits(mdplib::SOLVED_SSiPP)) {
                    bellmanUpdate(problem, s);
                }
            }
        } while (countExpanded != 0);
        while (true) {
            visited.clear();
            list<State*> stateStack;
            stateStack.push_back(s0);
            double error = 0.0;
            while (!stateStack.empty()) {
                State* s = stateStack.back();
                stateStack.pop_back();
                if (s->deadEnd() ||
                        problem->goal(s) ||
                        s->checkBits(mdplib::SOLVED_SSiPP ||
                        problem->overrideGoals()->count(s) > 0))
                    continue;
                if (!visited.insert(s).second)
                    continue;
                Action* prevAction = s->bestAction();
                if (prevAction == nullptr) {
                    // if it reaches this point it hasn't converged yet.
                    error = mdplib::dead_end_cost + 1;
                } else {
                    for (Successor sccr : problem->transition(s, prevAction))
                        stateStack.push_back(sccr.su_state);
                }
                error = std::max(error, bellmanUpdate(problem, s));
                if (prevAction != s->bestAction()) {
                    // it hasn't converged because the best action changed.
                    error = mdplib::dead_end_cost + 1;
                    break;
                }
            }
            if (error < epsilon_)
                return;
            if (error > mdplib::dead_end_cost) {
                break;  // BPSG changed, must expand tip nodes again
            }
        }
    }
}


}
