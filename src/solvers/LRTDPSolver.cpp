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
            dprint2("TRIAL ********* ", tmp);
            visited.push_front(tmp);
            if (problem_->goal(tmp))
                break;
            bellmanUpdate(problem_, tmp);
            if (tmp->bestAction() == nullptr) { // state is a dead-end
                break;
            }
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
        bool rv = true;
        std::list<mlcore::State*> open, closed;
        mlcore::StateSet openSet, closedSet;

        mlcore::State* tmp = s;
        if (!tmp->checkBits(mdplib::SOLVED)) {
            open.push_front(s);
            openSet.insert(s);
        }

        while (!open.empty()) {
            tmp = open.front();
            open.pop_front();
            openSet.erase(tmp);
            closed.push_front(tmp);
            closedSet.insert(tmp);

            if (residual(problem_, tmp) > epsilon_) {
                rv = false;
                continue;
            }

            /////////////////// THIS BLOCK IS A TEST ////////////////////////
            if (closedSet.size() > 1000) {
                break;
            }
            if (tmp->checkBits(mdplib::SOLVED)) {
                continue;
            }
            /////////////////// THIS BLOCK WAS A TEST ////////////////////////

            mlcore::Action* a = greedyAction(problem_, tmp);

            if (a == nullptr) {     // state is dead-end or goal, considered solved
                continue;
            }
            assert(problem_->applicable(tmp, a));
            std::list<mlcore::Successor> successors = problem_->transition(tmp, a);
            for (mlcore::Successor su : successors) {
                mlcore::State* next = su.su_state;
                if (!next->checkBits(mdplib::SOLVED)
                    && openSet.find(next) == openSet.end()
                    && closedSet.find(next) == closedSet.end())
                    open.push_front(next);
                    openSet.insert(next);
            }
        }

        if (rv && openSet.empty()) {
            for (mlcore::State* sc : closed)
                sc->setBits(mdplib::SOLVED);
        } else {
            while (!closed.empty()) {
                tmp = closed.front();
                closed.pop_front();
                bellmanUpdate(problem_, tmp);
            }
        }

        return rv;
    }

    mlcore::Action* LRTDPSolver::solve(mlcore::State* s0)
    {
        int trials = 0;
        while (!s0->checkBits(mdplib::SOLVED) && trials++ < maxTrials_)
            trial(s0);
    }

}

