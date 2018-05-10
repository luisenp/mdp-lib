#include "../../include/MDPLib.h"

#include "../../include/solvers/FLARESSolver.h"

using namespace mlcore;
using namespace std;

namespace mlsolvers
{

FLARESSolver::FLARESSolver(Problem* problem,
                         int maxTrials,
                         double epsilon,
                         double horizon,
                         bool optimal,
                         bool useProbsForDepth,
                         int maxTime) :
    problem_(problem),
    maxTrials_(maxTrials),
    epsilon_(epsilon),
    horizon_(horizon),
    optimal_(optimal),
    useProbsForDepth_(useProbsForDepth),
    maxTime_(maxTime)
{ }


void FLARESSolver::trial(State* s)
{
    State* currentState = s;
    list<State*> visited;
    double accumulated_cost = 0.0;
//                                                                                dprint("****************");
    while (!labeledSolved(currentState)) {
        if (problem_->goal(currentState))
            break;

        visited.push_front(currentState);
                                                                                double res = residual(problem_, currentState);
        bellmanUpdate(problem_, currentState);
                                                                                if (res < epsilon_ && residual(problem_, currentState) > epsilon_) {
                                                                                    cerr << "ooops!" << residual(problem_, currentState) << endl;
                                                                                }

        if (currentState->deadEnd()
            || accumulated_cost >= mdplib::dead_end_cost)
            break;

        mlcore::Action* greedy_action = greedyAction(problem_, currentState);
//                                                                                dprint(currentState, currentState->residualDistance(), greedy_action);
        accumulated_cost += problem_->cost(currentState, greedy_action);
                                                                                mdplib_tic();
        currentState = randomSuccessor(problem_, currentState, greedy_action);
                                                                                mdplib_toc();
                                                                                auto duration = mdplib_elapsed_nano();
                                                                                dprint("sample-successor", duration);
                                                                                if (labeledSolved(currentState))
                                                                                    dprint("solved-state");
    }
//                                                                                dprint("********");

    while (!visited.empty()) {
        currentState = visited.front();
        visited.pop_front();
                                                                                mdplib_tic();
        bool solved = checkSolved(currentState);
                                                                                mdplib_toc();
                                                                                auto duration = mdplib_elapsed_nano();
                                                                                dprint("check-solved", duration);
        if (!solved)
            break;
    }
}


bool FLARESSolver::labeledSolved(State* s)
{
    return (s->checkBits(mdplib::SOLVED) ||
        s->checkBits(mdplib::SOLVED_FLARES));
}


double FLARESSolver::computeNewDepth(Successor& su, double depth)
{
    if (useProbsForDepth_) {
        return depth + log(su.su_prob);
    } else {
        return depth + 1;
    }
}


bool FLARESSolver::checkSolved(State* s)
{
    list< pair<State*,double > > open, closed;

    State* currentState = s;
    if (!labeledSolved(currentState))
        open.push_front(make_pair(s, 0.0));
    else return true;

    bool rv = true;
    bool subgraphWithinSearchHorizon = optimal_ & true;
    while (!open.empty()) {
        pair<State*, double> pp = open.front();
        open.pop_front();
        currentState = pp.first;
        double depth = pp.second;

        if ( (useProbsForDepth_ && depth < 2 * log(horizon_)) ||
             (!useProbsForDepth_ && depth > 2 * horizon_) ) {
                subgraphWithinSearchHorizon = false;
                continue;
        }

        if (problem_->goal(currentState))
            continue;

        closed.push_front(pp);
        currentState->setBits(mdplib::CLOSED);

        Action* a = greedyAction(problem_, currentState);

        if (currentState->deadEnd())
            continue;

        if (residual(problem_, currentState) > epsilon_) {
            rv = false;
        }

        for (Successor su : problem_->transition(currentState, a)) {
            State* next = su.su_state;
            if (!labeledSolved(next) &&
                    !next->checkBits(mdplib::CLOSED)) {
                double newDepth = computeNewDepth(su, depth);
                open.push_front(make_pair(next, newDepth));
            } else if (next->checkBits(mdplib::SOLVED_FLARES) &&
                          !next->checkBits(mdplib::SOLVED)) {
                subgraphWithinSearchHorizon = false;
            }
        }
    }
//                                                                                dprint("  closed", closed.size());

    if (rv) {
        for (auto const & pp : closed) {
            pp.first->clearBits(mdplib::CLOSED);
            if (subgraphWithinSearchHorizon) {
//                                                                                dprint("  --flares SOLVED", pp.first);
                pp.first->setBits(mdplib::SOLVED_FLARES);
                pp.first->setBits(mdplib::SOLVED);
                depthSolved_.insert(pp.first);
            } else {
                if ( (useProbsForDepth_ && pp.second > log(horizon_)) ||
                     (!useProbsForDepth_ && pp.second <= horizon_) ) {
                                                                                dprint("solved a state", pp.second);
                        pp.first->setBits(mdplib::SOLVED_FLARES);
                        depthSolved_.insert(pp.first);
//                                                                                dprint("  --flares depth-solved", pp.first);
                }
            }
        }
    } else {
        while (!closed.empty()) {
            pair<State*, double> pp;
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
    auto begin = std::chrono::high_resolution_clock::now();
    while (!labeledSolved(s0) && trials++ < maxTrials_) {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::
            duration_cast<std::chrono::milliseconds>(end-begin).count();
        if (maxTime_ > -1 && duration >= maxTime_)
            break;
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

