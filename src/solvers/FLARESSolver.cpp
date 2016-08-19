#include "../../include/MDPLib.h"

#include "../../include/solvers/FLARESSolver.h"

using namespace mlcore;
using namespace std;

namespace mlsolvers
{

FLARESSolver::FLARESSolver(Problem* problem,
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


void FLARESSolver::trial(State* s)
{
    State* currentState = s;
    list<State*> visited;
    int depth = 0;
    while (!labeledSolved(currentState)) {
        if (problem_->goal(currentState))
            break;

        visited.push_front(currentState);
        depth++;
                                                                                double res = residual(problem_, currentState);
        bellmanUpdate(problem_, currentState);
                                                                                if (res < epsilon_ && residual(problem_, currentState) > epsilon_) {
                                                                                    cerr << "ooops!" << residual(problem_, currentState) << endl;
                                                                                }

        if (currentState->deadEnd() || depth > 1000000)
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


bool FLARESSolver::labeledSolved(State* s)
{
    return (s->checkBits(mdplib::SOLVED) ||
        s->checkBits(mdplib::SOLVED_FLARES));
}


bool FLARESSolver::checkSolved(State* s)
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

        if (problem_->goal(currentState) || currentState->deadEnd())
            continue;

        closed.push_front(pp);
        currentState->setBits(mdplib::CLOSED);


        Action* a = greedyAction(problem_, currentState);

        if (residual(problem_, currentState) > epsilon_)
            rv = false;

        for (Successor su : problem_->transition(currentState, a)) {
            State* next = su.su_state;
            if (!labeledSolved(next) &&
                    !next->checkBits(mdplib::CLOSED)) {
                open.push_front(make_pair(next, depth + 1));
            } else if (next->checkBits(mdplib::SOLVED_FLARES) &&
                          !next->checkBits(mdplib::SOLVED)) {
                subgraphWithinSearchHorizon = false;
            }
        }
    }

    if (rv) {
        for (auto const & pp : closed) {
            pp.first->clearBits(mdplib::CLOSED);
            if (subgraphWithinSearchHorizon) {
                pp.first->setBits(mdplib::SOLVED_FLARES);
                pp.first->setBits(mdplib::SOLVED);
                depthSolved_.insert(pp.first);
            } else if (pp.second <= horizon_) {
                pp.first->setBits(mdplib::SOLVED_FLARES);
                depthSolved_.insert(pp.first);
            }
        }
    } else {
        while (!closed.empty()) {
            pair<State*, int> pp;
            pp = closed.front();
            closed.pop_front();
            pp.first->clearBits(mdplib::CLOSED);
                                                                                double res = residual(problem_, pp.first);
            bellmanUpdate(problem_, pp.first);
                                                                                if (res < epsilon_ && residual(problem_, pp.first) > epsilon_) {
                                                                                    cerr << "ooops!" << residual(problem_, pp.first) << endl;
                                                                                }
        }
    }

    return rv;
}


Action* FLARESSolver::solve(State* s0)
{
    if (optimal_)
        return solveOptimally(s0);
    return solveApproximate(s0);
}


Action* FLARESSolver::solveApproximate(State* s0)
{
    int trials = 0;
    while (!labeledSolved(s0) && trials++ < maxTrials_) {
        trial(s0);
    }
    return s0->bestAction();
}


Action* FLARESSolver::solveOptimally(State* s0)
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
        for (State* tmp : depthSolved_)
            tmp->clearBits(mdplib::SOLVED_FLARES);
        depthSolved_.clear();
    }
    return s0->bestAction();
}

}

