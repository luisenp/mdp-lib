#include "../../include/solvers/solver.h"
#include "../../include/solvers/LAOStarSolver.h"

#include "../../include/util/general.h"

namespace mlsolvers
{
    mlcore::Action* LAOStarSolver::solve(mlcore::State* s0)
    {
        int totalExpanded = 0;
        int countExpanded = 0;
        double error = mdplib::dead_end_cost;
        while (true) {
            do {
                visited.clear();
                countExpanded = expand(s0, 0);
                totalExpanded += countExpanded;
            } while (countExpanded != 0);

                dprint2(s0->cost(), totalExpanded);

            while (true) {
                visited.clear();
                error = testConvergence(s0, 0);
                if (error < epsilon_)
                    return s0->bestAction();
                if (error > mdplib::dead_end_cost) {
                    break;  // BPSG changed, must expand tip nodes again
                }
            }
        }
    }

    int LAOStarSolver::expand(mlcore::State* s, int level)
    {
        if (!visited.insert(s).second)
            return 0;

        int cnt = 0;

        if (s->deadEnd() || problem_->goal(s))
            return 0;

        if (s->bestAction() == nullptr) {   // this means state has not been expanded
            bellmanUpdate(problem_, s);
            cnt = 1;
        } else {
            mlcore::Action* a = s->bestAction();
            std::list<mlcore::Successor> successors = problem_->transition(s, a);
            for (mlcore::Successor sccr : successors)
                cnt += expand(sccr.su_state, level + 1);
        }
        bellmanBackup(problem_, s);
        return cnt;
    }

    double LAOStarSolver::testConvergence(mlcore::State* s, int level)
    {
        double error = 0.0;

        if (s->deadEnd() || problem_->goal(s))
            return 0.0;

        if (!visited.insert(s).second)
            return 0.0;

        mlcore::Action* prevAction = s->bestAction();
        if (prevAction == nullptr) {
            // hasn't converged because a state in BPSG doesn't have an action ready
            return mdplib::dead_end_cost + 1;
        } else {
            std::list<mlcore::Successor> successors = problem_->transition(s, prevAction);
            for (mlcore::Successor sccr : successors)
                error =  std::max(error, testConvergence(sccr.su_state, level + 1));
        }

        error = std::max(error, bellmanUpdate(problem_, s));
        if (prevAction == s->bestAction())
            return error;
        return mdplib::dead_end_cost + 2; // hasn't converged because the best action changed
    }

}

