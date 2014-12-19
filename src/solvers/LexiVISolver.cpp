#include <list>
#include <climits>
#include <cmath>

#include "../../include/solvers/solver.h"
#include "../../include/solvers/LexiVISolver.h"
#include "../../include/lexi/lexi_problem.h"
#include "../../include/lexi/lexi_state.h"
#include "../../include/util/general.h"

namespace mlsolvers
{
    VISolver::VISolver(mlcore::Problem* problem, int maxIter, double tol)
    {
        problem_ = problem;
        maxIter_ = maxIter;
        tol_ = tol;
    }

    mlcore::Action* VISolver::solve(mlcore::State* s0)
    {
        for (int i = 0; i < maxIter_; i++) {
            double maxResidual = 0.0;
            for (mlcore::State* s : problem_->states()) {
                double residual = bellmanUpdate(problem_, s, 1.0);
                if (residual > maxResidual)
                    maxResidual = residual;
            }
            if (maxResidual < tol_)
                return nullptr;
        }
    }
}
