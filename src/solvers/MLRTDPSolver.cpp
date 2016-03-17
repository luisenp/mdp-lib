#include "../../include/MDPLib.h"

#include "../../include/solvers/MLRTDPSolver.h"

using namespace mlcore;
using namespace std;

namespace mlsolvers
{

MLRTDPSolver::MLRTDPSolver(Problem* problem,
                         int maxTrials,
                         double epsilon,
                         int horizon) :
    problem_(problem),
    maxTrials_(maxTrials),
    epsilon_(epsilon),
    horizon_(horizon)
{ }


void MLRTDPSolver::trial(State* s)
{
    State* currentState = s;
    list<State*> visited;
    while (!labeledSolved(currentState)) {
        visited.push_front(currentState);
        if (problem_->goal(currentState))
            break;

        bellmanUpdate(problem_, currentState);

        if (currentState->deadEnd())
            break;

        currentState = randomSuccessor(problem_,
                                       currentState,
                                       greedyAction(problem_, currentState));
    }

    while (!visited.empty()) {
        currentState = visited.front();
        visited.pop_front();
        if (!checkSolved(currentState))
            break;
    }
}


bool MLRTDPSolver::labeledSolved(State* s)
{
    return (s->checkBits(mdplib::SOLVED) || (depthSolved_.count(s) > 0));
}


bool MLRTDPSolver::checkSolved(State* s)
{
    list< pair<State*,int > > open, closed;

    State* currentState = s;
    if (!labeledSolved(currentState))
        open.push_front(make_pair(s, 0));
    else return true;

    bool rv = true;
    bool subgraphWithinSearchHorizon = true;
    while (!open.empty()) {
        pair<State*, int> pp = open.front();
        open.pop_front();
        currentState = pp.first;
        int depth = pp.second;

        if (depth > 2 * horizon_) {
            subgraphWithinSearchHorizon = false;
            continue;
        }
//                                                                                dprint3("  ", currentState, s);

        closed.push_front(pp);
        currentState->setBits(mdplib::CLOSED_MLRTDP);

        Action* a = greedyAction(problem_, currentState);

        if (problem_->goal(currentState))
            continue;

        if (currentState->deadEnd()) {
            rv = false;
            continue;
        }

        if (residual(problem_, currentState) > epsilon_)
            rv = false;

        for (Successor su : problem_->transition(currentState, a)) {
            State* next = su.su_state;
//                                                                                dprint5("    next",
//                                                                                        next,
//                                                                                        currentState,
//                                                                                        next->checkBits(mdplib::CLOSED_MLRTDP),
//                                                                                        next->bits());
            if (!labeledSolved(next) &&
                    !next->checkBits(mdplib::CLOSED_MLRTDP)) {
//                                                                                dprint1("    will-recurse");
                open.push_front(make_pair(next, depth + 1));
            } else if (depthSolved_.count(next) > 0 &&
                          !next->checkBits(mdplib::SOLVED)) {
//                                                                                dprint2("    labeled but not opt", next);
                subgraphWithinSearchHorizon = false;
            }
        }
    }

    if (rv) {
        for (auto const & pp : closed) {
            pp.first->clearBits(mdplib::CLOSED_MLRTDP);
            if (subgraphWithinSearchHorizon) {
                depthSolved_.insert(pp.first);
                pp.first->setBits(mdplib::SOLVED);

//                                                                                StateSet reach, tips;
//                                                                                getBestPartialSolutionGraph(problem_, pp.first, reach);
//                                                                                dprint5("    solved", pp.first, "size", reach.size(), s);
//                                                                                dprint2("    solved", pp.first);
//                                                                                for (auto foo : reach) {
//                                                                                    if (residual(problem_, foo) > epsilon_) {
//                                                                                        dprint2("shouldnt have been cuz of", foo);
//                                                                                        exit(-1);
//                                                                                    }
//                                                                                }
            } else if (pp.second <= horizon_) {
                depthSolved_.insert(pp.first);
//                                                                                dprint2("depth solved", pp.first);
            }
        }
    } else {
        while (!closed.empty()) {
            pair<State*, int> pp;
            pp = closed.front();
            closed.pop_front();
            pp.first->clearBits(mdplib::CLOSED_MLRTDP);
            bellmanUpdate(problem_, pp.first);
        }
    }

    return rv;
}


Action* MLRTDPSolver::solve(State* s0)
{
    return solveOptimally(s0);
//    int trials = 0;
//    while (!labeledSolved(s0) && trials++ < maxTrials_) {
//        trial(s0);
//    }
//    isClosedPolicy(s0);
//    return s0->bestAction();
}


Action* MLRTDPSolver::solveOptimally(State* s0)
{
    horizon_ = 0;
    while (true) {
        int trials = 0;
        while (!labeledSolved(s0) && trials++ < maxTrials_) {
            trial(s0);
        }
        if (s0->checkBits(mdplib::SOLVED))
            break;
        horizon_ = 2 * horizon_ + 1;
        depthSolved_.clear();
    }
                                                                                dprint3("horizon, solved-size", horizon_, depthSolved_.size());
    return s0->bestAction();
}


bool MLRTDPSolver::isClosedPolicy(State* s0)
{
    int cnt = 0;
    list<State *> stateQueue;
    stateQueue.push_front(s0);
    StateSet visited;
    bool isClosed = true;
    while (!stateQueue.empty()) {
        State* state = stateQueue.back();
        stateQueue.pop_back();
        if (!visited.insert(state).second)
            continue;
        if (problem_->goal(state))
            continue;
        if (!labeledSolved(state)) {
            cnt++;
            isClosed = false;
            break;
        }
        Action* a = greedyAction(problem_, state);
        for (Successor& sccr : problem_->transition(state, a)) {
            stateQueue.push_front(sccr.su_state);
        }
    }
    dprint2("NOT SOLVED", double(cnt) / visited.size());
    return isClosed;
}

}

