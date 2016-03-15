#include "../../include/MDPLib.h"
#include "../../include/State.h"

#include "../../include/domains/WrapperProblem.h"

#include "../../include/solvers/Solver.h"
#include "../../include/solvers/LAOStarSolver.h"
#include "../../include/solvers/LRTDPSolver.h"
#include "../../include/solvers/SSiPPSolver.h"

using namespace mlcore;
using namespace std;

namespace mlsolvers
{

Action* SSiPPSolver::solveOriginal(State* s0)
{
    StateSet reachableStates, tipStates;
    getReachableStates(problem_, reachableStates, tipStates, t_);
    WrapperProblem* wrapper = new WrapperProblem(problem_);
    wrapper->overrideStates(&reachableStates);
    wrapper->overrideGoals(&tipStates);
    LRTDPSolver lrtdpSolver(wrapper, 100000, epsilon_);
    lrtdpSolver.solve(s0);
    wrapper->cleanup();
    return s0->bestAction();
}


Action* SSiPPSolver::solveLabeled(State* s0)
{
    while (!s0->checkBits(mdplib::SOLVED_SSiPP)) {
        State* currentState = s0;
        list<State*> visited;
        while (!currentState->checkBits(mdplib::SOLVED_SSiPP)) {
//                                                                                dprint3(currentState,
//                                                                                        problem_->goal(currentState),
//                                                                                        currentState->deadEnd());
            visited.push_front(currentState);
            if (problem_->goal(currentState))
                break;

            StateSet reachableStates, tipStates;
            // This makes getReachableStates start the search at currentState.
            reachableStates.insert(currentState);
            getReachableStates(problem_, reachableStates, tipStates, t_);

            WrapperProblem wrapper(problem_);
            wrapper.overrideStates(&reachableStates);
            wrapper.overrideGoals(&tipStates);

            // We use LAO* to avoid labeling states as solved when solving
            // another Short-Sighted SSP.
//            bellmanUpdate(problem_, currentState);
            LAOStarSolver solver(&wrapper);
            solver.solve(currentState);
//                                                                                dprint3(currentState->cost(),
//                                                                                        currentState->bestAction(),
//                                                                                        currentState->checkBits(mdplib::SOLVED_SSiPP));

            if (currentState->deadEnd())
                break;

            currentState = randomSuccessor(problem_,
                                           currentState,
                                           greedyAction(problem_,
                                                        currentState));
//                                                                                dprint3("  ",
//                                                                                        currentState,
//                                                                                        currentState->checkBits(mdplib::SOLVED_SSiPP));

            wrapper.cleanup();
        }
        while (!visited.empty()) {
            currentState = visited.front();
            visited.pop_front();
            if (!checkSolved(currentState))
                break;
        }
    }
}


bool SSiPPSolver::checkSolved(mlcore::State* s)
{
    std::list<mlcore::State*> open, closed;

    mlcore::State* tmp = s;
    if (!tmp->checkBits(mdplib::SOLVED_SSiPP)) {
        open.push_front(s);
        s->setBits(mdplib::CLOSED_SSiPP);
    }

    bool rv = true;
    while (!open.empty()) {
        tmp = open.front();
        open.pop_front();
        closed.push_front(tmp);

        mlcore::Action* a = greedyAction(problem_, tmp);
        tmp->setBestAction(a);

        if (problem_->goal(tmp)) {
//                                                                                dprint2("goal", tmp);
            continue;
        }

        if (tmp->deadEnd()) {
            rv = false;
            continue;
        }

        if (residual(problem_, tmp) > epsilon_)
            rv = false;

        for (mlcore::Successor su : problem_->transition(tmp, a)) {
            mlcore::State* next = su.su_state;
            if (!next->checkBits(mdplib::SOLVED_SSiPP) &&
                !next->checkBits(mdplib::CLOSED_SSiPP)) {
                open.push_front(next);
                next->setBits(mdplib::CLOSED_SSiPP);
            }
        }
    }

    if (rv) {
        for (mlcore::State* sc : closed) {
//                                                                                dprint2("solved", sc);
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

}
