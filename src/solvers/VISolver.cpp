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
        Rational maxResidual(0);
        for (State* s : problem_->states()) {
            Rational residual = bellmanUpdate(problem_, s);
            if (residual > maxResidual)
                maxResidual = residual;
        }
        if (maxResidual < tol)
            return;
    }
}
