#include "../../include/solvers/LRTDPSolver.h"

namespace mlsolvers
{

LRTDPSolver::LRTDPSolver(mlcore::Problem* problem,
                         int maxTrials,
                         double epsilon) :
    problem_(problem), maxTrials_(maxTrials), epsilon_(epsilon)
{ }


void LRTDPSolver::trial(mlcore::State* s) {
    mlcore::State* tmp = s;
    std::list<mlcore::State*> visited;
    while (!tmp->checkBits(mdplib::SOLVED)) {
        if (problem_->goal(tmp))
            break;

        visited.push_front(tmp);

        bellmanUpdate(problem_, tmp);

        if (tmp->deadEnd())
            break;

                                                                                auto begin = std::chrono::high_resolution_clock::now();
        tmp = randomSuccessor(problem_, tmp, tmp->bestAction());
                                                                                auto end = std::chrono::high_resolution_clock::now();
                                                                                auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count();
                                                                                cnt_samples_++;
                                                                                total_time_samples_ += duration;
    }

    while (!visited.empty()) {
        tmp = visited.front();
        visited.pop_front();
                                                                                auto begin = std::chrono::high_resolution_clock::now();
        bool solved = checkSolved(tmp);
                                                                                auto end = std::chrono::high_resolution_clock::now();
                                                                                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-begin).count();
                                                                                cnt_check_++;
                                                                                total_time_check_ += duration;
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

        closed.push_front(tmp);
        tmp->setBits(mdplib::CLOSED);

        if (residual(problem_, tmp) > epsilon_) {
            rv = false;
            // The original paper includes this line, but the algorithm
            // seems to work significantly faster without this
            //  continue;
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
        }
    } else {
        while (!closed.empty()) {
            tmp = closed.front();
            closed.pop_front();
            tmp->clearBits(mdplib::CLOSED);
            bellmanUpdate(problem_, tmp);
        }
    }

    return rv;
}


mlcore::Action* LRTDPSolver::solve(mlcore::State* s0)
{
    int trials = 0;
    while (!s0->checkBits(mdplib::SOLVED) && trials++ < maxTrials_) {
        trial(s0);
    }
                                                                                dprint(cnt_samples_, double(total_time_samples_) / cnt_samples_);
                                                                                dprint(cnt_check_, double(total_time_check_) / cnt_check_);
                                                                                dprint(trials);
    return s0->bestAction();
}

}

