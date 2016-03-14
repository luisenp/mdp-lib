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
    State* tmp = s;
    list<State*> visited;
    while (!tmp->checkBits(mdplib::SOLVED)) {
        visited.push_front(tmp);
        if (problem_->goal(tmp))
            break;

        bellmanUpdate(problem_, tmp);

        if (tmp->deadEnd())
            break;

        tmp = randomSuccessor(problem_, tmp, tmp->bestAction());
    }

    while (!visited.empty()) {
        tmp = visited.front();
        visited.pop_front();
        if (!checkSolved(tmp))
            break;
    }
}


bool MLRTDPSolver::checkSolved(State* s)
{
    list< pair<State*,int > > open, closed;

    State* tmp = s;
    if (!tmp->checkBits(mdplib::SOLVED)) {
        open.push_front(make_pair(s, 0));
        s->setBits(mdplib::CLOSED);
    }

    bool rv = true;
    while (!open.empty()) {
        pair<State*, int> pp = open.front();
        open.pop_front();
        tmp = pp.first;
        int depth = pp.second;
        closed.push_front(pp);

        Action* a = greedyAction(problem_, tmp);

        if (problem_->goal(tmp))
            continue;

        if (tmp->deadEnd()) {
            rv = false;
            continue;
        }

        if (residual(problem_, tmp) > epsilon_)
            rv = false;

        for (Successor su : problem_->transition(tmp, a)) {
            State* next = su.su_state;
            if (!next->checkBits(mdplib::SOLVED) &&
                !next->checkBits(mdplib::CLOSED) &&
                depth <= 2 * horizon_) {
                open.push_front(make_pair(next, depth + 1));
                next->setBits(mdplib::CLOSED);
            }
        }
    }

    if (rv) {
        for (auto const & pp : closed) {
            if (pp.second <= horizon_)
                pp.first->setBits(mdplib::SOLVED);
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
    int trials = 0;
    while (!s0->checkBits(mdplib::SOLVED) && trials++ < maxTrials_) {
        trial(s0);
    }
}

}

