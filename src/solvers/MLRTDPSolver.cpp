#include "../../include/MDPLib.h"

#include "../../include/solvers/MLRTDPSolver.h"

using namespace mlcore;
using namespace std;

namespace mlsolvers
{

MLRTDPSolver::MLRTDPSolver(Problem* problem,
                         int maxTrials,
                         double epsilon,
                         int horizon,
                         bool optimal) :
    problem_(problem),
    maxTrials_(maxTrials),
    epsilon_(epsilon),
    horizon_(horizon),
    optimal_(optimal)
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
    bool subgraphWithinSearchHorizon = optimal_ & true;
    while (!open.empty()) {
        pair<State*, int> pp = open.front();
        open.pop_front();
        currentState = pp.first;
        int depth = pp.second;

        if (depth > 2 * horizon_) {
            subgraphWithinSearchHorizon = false;
            continue;
        }

        closed.push_front(pp);
        currentState->setBits(mdplib::CLOSED);

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
            if (!labeledSolved(next) &&
                    !next->checkBits(mdplib::CLOSED)) {
                open.push_front(make_pair(next, depth + 1));
            } else if (depthSolved_.count(next) > 0 &&
                          !next->checkBits(mdplib::SOLVED)) {
                subgraphWithinSearchHorizon = false;
            }
        }
    }

    if (rv) {
        for (auto const & pp : closed) {
            pp.first->clearBits(mdplib::CLOSED);
            if (subgraphWithinSearchHorizon) {
                depthSolved_.insert(pp.first);
                pp.first->setBits(mdplib::SOLVED);
            } else if (pp.second <= horizon_) {
                depthSolved_.insert(pp.first);
            }
        }
    } else {
        while (!closed.empty()) {
            pair<State*, int> pp;
            pp = closed.front();
            closed.pop_front();
            pp.first->clearBits(mdplib::CLOSED);
            bellmanUpdate(problem_, pp.first);
        }
    }

    return rv;
}


Action* MLRTDPSolver::solve(State* s0)
{
    if (optimal_)
        return solveOptimally(s0);
    return solveApproximate(s0);
}


Action* MLRTDPSolver::solveApproximate(State* s0)
{
    int trials = 0;
    while (!labeledSolved(s0) && trials++ < maxTrials_)
        trial(s0);
    return s0->bestAction();
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
    return s0->bestAction();
}

}

