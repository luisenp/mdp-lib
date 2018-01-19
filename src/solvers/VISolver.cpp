#include <list>
#include <climits>
#include <cmath>

#include "../../include/solvers/Solver.h"
#include "../../include/solvers/VISolver.h"
#include "../../include/State.h"
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
                if (problem_->goal(s))
                    continue;
                double residual = bellmanUpdate(problem_, s);
                maxResidual = std::max(residual, maxResidual);
            }
            if (maxResidual < tol_)
                return nullptr;
        }
    }
}
