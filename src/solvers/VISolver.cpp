#include <list>
#include <climits>
#include <cmath>

#include "../../include/solvers/solver.h"
#include "../../include/solvers/VISolver.h"
#include "../../include/state.h"
#include "../../include/util/general.h"

VISolver::VISolver(Problem* problem)
{
    problem_ = problem;
}

void VISolver::solve(int maxIter, Rational tol)
{
    for (int i = 0; i < maxIter; i++) {
        Rational residual(0);
        for (State* s : problem_->states()) {
            std::pair<Rational, Action*> best = bellmanBackup(problem_, s);
            Rational diff(fabs(s->cost().value() - best.first.value()));
            if (diff > residual)
                residual = diff;
            s->setCost(best.bb_cost);
            s->setBestAction(best.bb_action);
        }
        if (residual < tol)
            return;
    }
}
