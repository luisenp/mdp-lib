#include "../../include/solvers/solver.h"
#include "../../include/solvers/LRTDPSolver.h"

namespace mlsolvers
{
    LRTDPSolver::LRTDPSolver(mlcore::Problem* problem)
    {
        problem_ = problem;
    }

    void LRTDPSolver::trial(Rational epsilon)
    {
        mlcore::State* tmp = problem_->initialState();
        std::list<mlcore::State*> visited;
        while (!tmp->checkBits(mdplib::SOLVED)) {
            visited.push_front(tmp);
            if (problem_->goal(tmp))
                break;
            bellmanUpdate(problem_, tmp);
            tmp = randomSuccessor(problem_, tmp, tmp->bestAction());
        }

        while (!visited.empty()) {
            tmp = visited.front();
            visited.pop_front();
            if (!checkSolved(tmp, epsilon))
                break;
        }
    }

    bool LRTDPSolver::checkSolved(mlcore::State* s, Rational epsilon)
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

            if (residual(problem_, tmp) > epsilon) {
                rv = false;
                continue;
            }

            mlcore::Action* a = greedyAction(problem_, tmp);
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

        if (rv) {
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

    void LRTDPSolver::solve(mlcore::State* s0, int maxTrials, Rational epsilon)
    {
        int trials = 0;
        while (!s0->checkBits(mdplib::SOLVED) && trials++ < maxTrials)
            trial(epsilon);
    }

}

