#include "../../include/solvers/solver.h"
#include "../../include/solvers/LAOStarSolver.h"

#include "../../include/util/general.h"

void LAOStarSolver::solve(State* s0, Rational epsilon)
{
    int totalExpanded = 0;
    int countExpanded = 0;
    double error = mdplib::dead_end_cost;
    while (true) {
        do {
            visited.clear();
            countExpanded = expand(s0, 0);
            totalExpanded += countExpanded ;
        } while (countExpanded != 0);

        while (true) {
            visited.clear();
            error = testConvergence(s0, 0);
            if (error < epsilon.value())
                return;
            if (error > mdplib::dead_end_cost)
                break;  // BPSG changed, must expand tip nodes again
        }
    }
}

int LAOStarSolver::expand(State* s, int level)
{
    if (!visited.insert(s).second)
        return 0;

    int cnt = 0;
    if (s->bestAction() == nullptr) {   // this means state has not been expanded
        bellmanUpdate(problem_, s);
        cnt = 1;
    } else {
        if (problem_->goal(s))
            return 0;
        Action* a = s->bestAction();
        std::list<Successor> successors = problem_->transition(s, a);
        for (Successor sccr : successors)
            cnt += expand(sccr.su_state, level + 1);
    }
    bellmanBackup(problem_, s);
    return cnt;
}

double LAOStarSolver::testConvergence(State* s, int level)
{
    double error = 0.0;
    if (!visited.insert(s).second)
        return (s->bestAction() == nullptr) ? mdplib::dead_end_cost + 1 : 0.0;

    Action* prevAction = s->bestAction();
    if (prevAction == nullptr) {
        // hasn't converged because a state in BPSG doesn't have an action ready
        return mdplib::dead_end_cost + 1;
    } else {
        if (problem_->goal(s))
            return 0.0;
        std::list<Successor> successors = problem_->transition(s, prevAction);
        for (Successor sccr : successors)
            error =  std::max(error, testConvergence(sccr.su_state, level + 1));
    }

    error = std::max(error, bellmanUpdate(problem_, s).value());
    if (prevAction == s->bestAction())
        return error;
    return mdplib::dead_end_cost + 1; // hasn't converged because the best action changed
}
