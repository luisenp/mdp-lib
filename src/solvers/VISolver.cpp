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
        maxTime_ = -1;
    }

    mlcore::Action* VISolver::solve(mlcore::State* s0)
    {
        auto beginTime = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < maxIter_; i++) {
            double maxResidual = 0.0;
            for (mlcore::State* s : problem_->states()) {
                if (problem_->goal(s)) {
                    continue;
                }
                double residual = bellmanUpdate(problem_, s);
                maxResidual = std::max(residual, maxResidual);

                auto endTime = std::chrono::high_resolution_clock::now();
                auto timeElapsed = std::chrono::duration_cast<
                    std::chrono::milliseconds>(endTime - beginTime).count();
                if (maxTime_ > -1 && timeElapsed > maxTime_)
                    return nullptr;
            }
            if (maxResidual < tol_)
                return nullptr;
        }
        return nullptr;
    }
}
