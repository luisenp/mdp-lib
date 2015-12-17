#include <list>
#include <climits>
#include <cmath>

#include "../../include/solvers/solver.h"
#include "../../include/solvers/VISolver.h"
#include "../../include/state.h"
#include "../../include/util/general.h"

namespace mlsolvers
{
    VISolver::VISolver(mlcore::Problem* problem, int maxIter, double tol)
    {
        mdplib_debug = true;
        dprint1("VISOLVER");
        problem_ = problem;
        maxIter_ = maxIter;
        tol_ = tol;
    }

    mlcore::Action* VISolver::solve(mlcore::State* s0)
    {
        mdplib_debug = true;
        dprint2("solving", maxIter_);
        for (int i = 0; i < maxIter_; i++) {
            double maxResidual = 0.0;
            for (mlcore::State* s : problem_->states()) {
                double residual = bellmanUpdate(problem_, s);
                if (residual > maxResidual)
                    maxResidual = residual;
            }
                                                std:: cout << maxResidual << std::endl;
            if (maxResidual < tol_)
                return nullptr;
        }
    }
}
