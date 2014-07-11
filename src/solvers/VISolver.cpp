#include <list>
#include <climits>
#include <cmath>

#include "../../include/solvers/solver.h"
#include "../../include/solvers/VISolver.h"
#include "../../include/state.h"
#include "../../include/util/general.h"

namespace mlsolvers
{
    VISolver::VISolver(mlcore::Problem* problem)
    {
        problem_ = problem;
    }

    void VISolver::solve(int maxIter, double tol)
    {
        for (int i = 0; i < maxIter; i++) {
            double maxResidual = 0.0;
            for (mlcore::State* s : problem_->states()) {
                double residual = bellmanUpdate(problem_, s);
                if (residual > maxResidual)
                    maxResidual = residual;
            }
            if (maxResidual < tol)
                return;
        }
    }
}
