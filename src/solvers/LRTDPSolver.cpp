#include "../../include/solvers/LRTDPSolver.h"

namespace mlsolvers
{

LRTDPSolver::LRTDPSolver(mlcore::Problem* problem,
                         int maxTrials,
                         double epsilon,
                         int maxTime,
                         bool dont_label) :
    problem_(problem),
    maxTrials_(maxTrials),
    epsilon_(epsilon),
    maxTime_(maxTime),
    dont_label_(dont_label)
{ }


bool LRTDPSolver::ranOutOfTime() {
    if (maxTime_ > -1) {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto timeElapsed = std::chrono::duration_cast<
            std::chrono::milliseconds>(endTime - beginTime_).count();
        if (timeElapsed > maxTime_)
            return true;
    }
    return false;
}

void LRTDPSolver::trial(mlcore::State* s) {
    mlcore::State* tmp = s;
    std::list<mlcore::State*> visited;
    double accumulated_cost = 0.0;
    while (!tmp->checkBits(mdplib::SOLVED)) {
        if (problem_->goal(tmp) || accumulated_cost > mdplib::dead_end_cost)
            break;

        visited.push_front(tmp);

        bellmanUpdate(problem_, tmp);

        if (tmp->deadEnd())
            break;

        accumulated_cost += problem_->cost(tmp, tmp->bestAction());
        tmp = randomSuccessor(problem_, tmp, tmp->bestAction());
    }

    if (dont_label_)
        return;

    while (!visited.empty()) {
        tmp = visited.front();
        visited.pop_front();
        bool solved = checkSolved(tmp);
        if (!solved) break;
    }
}


bool LRTDPSolver::checkSolved(mlcore::State* s)
{
    std::list<mlcore::State*> open, closed;

    mlcore::State* tmp = s;
    if (!tmp->checkBits(mdplib::SOLVED)) {
        open.push_front(s);
    }

    bool rv = true;
    while (!open.empty()) {
        tmp = open.front();
        open.pop_front();

        if (problem_->goal(tmp))
            continue;

        mlcore::Action* a = greedyAction(problem_, tmp);

        if (tmp->deadEnd())
            continue;

        if (ranOutOfTime())
            return false;

        closed.push_front(tmp);
        tmp->setBits(mdplib::CLOSED);

        if (residual(problem_, tmp) > epsilon_) {
            rv = false;
            // The original paper includes the following line, but the algorithm
            // seems to work significantly faster without it
            /*  continue; */
        }

        for (mlcore::Successor su : problem_->transition(tmp, a)) {
            mlcore::State* next = su.su_state;
            if (!next->checkBits(mdplib::SOLVED) &&
                !next->checkBits(mdplib::CLOSED)) {
                open.push_front(next);
            }
        }
    }

    if (rv) {
        for (mlcore::State* sc : closed) {
            sc->setBits(mdplib::SOLVED);
            sc->clearBits(mdplib::CLOSED);
            sc->setBestAction(greedyAction(problem_, sc));
        }
    } else {
        while (!closed.empty()) {
            tmp = closed.front();
            closed.pop_front();
            tmp->clearBits(mdplib::CLOSED);
            bellmanUpdate(problem_, tmp);
            if (ranOutOfTime())
                return false;
            }
    }

    return rv;
}


mlcore::Action* LRTDPSolver::solve(mlcore::State* s0)
{
    int trials = 0;
    beginTime_ = std::chrono::high_resolution_clock::now();
    while (!s0->checkBits(mdplib::SOLVED) && trials++ < maxTrials_) {
        trial(s0);
        if (ranOutOfTime()) {
            return greedyAction(problem_, s0);
        }
    }
    return s0->bestAction();
}

}

