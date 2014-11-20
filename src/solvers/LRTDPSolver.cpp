#include "../../include/solvers/LRTDPSolver.h"

namespace mlsolvers
{
    LRTDPSolver::LRTDPSolver(mlcore::Problem* problem, int maxTrials, double epsilon)
    {
        problem_ = problem;
        maxTrials_ = maxTrials;
        epsilon_ = epsilon;
    }

    void LRTDPSolver::trial(mlcore::State* s)
    {
        mlcore::State* tmp = s;
        std::list<mlcore::State*> visited;
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

    bool LRTDPSolver::checkSolved(mlcore::State* s)
    {
        std::list<mlcore::State*> open, closed;

        mlcore::State* tmp = s;
        if (!tmp->checkBits(mdplib::SOLVED)) {
            open.push_front(s);
            s->setBits(mdplib::CLOSED);
        }

        bool rv = true;
        while (!open.empty()) {
            tmp = open.front();
            open.pop_front();
            closed.push_front(tmp);

            mlcore::Action* a = greedyAction(problem_, tmp);

            if (problem_->goal(tmp) || tmp->deadEnd())
                continue;

            if (residual(problem_, tmp) > epsilon_)
                rv = false;


            for (mlcore::Successor su : problem_->transition(tmp, a)) {
                mlcore::State* next = su.su_state;
                if (!next->checkBits(mdplib::SOLVED) && !next->checkBits(mdplib::CLOSED)) {
                    open.push_front(next);
                    next->setBits(mdplib::CLOSED);
                }
            }
        }

        if (rv) {
            for (mlcore::State* sc : closed) {
                sc->setBits(mdplib::SOLVED);
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
//            checkSolved(s0);
            trial(s0);
//            dprint1("*****************************");   dsleep(1000);
        }
    }

}

