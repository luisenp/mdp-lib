#include <list>
#include <climits>
#include <cmath>

#include "../../include/solvers/solver.h"
#include "../../include/solvers/VISolver.h"
#include "../../include/state.h"
#include "../../include/util/general.h"

#include "../../include/domains/gridworld/GridWorldAction.h"

VISolver::VISolver(Problem* problem)
{
    problem_ = problem;
}

void VISolver::solve(int maxIter, Rational tol)
{
    for (int i = 0; i < maxIter; i++) {
        Rational residual(0);
        for (State* s : problem_->states()) {
            Rational bestQ = bellmanBackup(problem_, s);
            Rational diff(fabs(s->cost().value() - bestQ.value()));
            if (diff > residual)
                residual = diff;
            s->setCost(bestQ);
        }
        if (residual < tol)
            return;
    }
}
