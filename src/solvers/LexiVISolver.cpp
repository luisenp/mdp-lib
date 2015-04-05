#include <list>
#include <climits>
#include <cmath>

#include "../../include/lexi/domains/MORacetrackState.h"

#include "../../include/solvers/solver.h"
#include "../../include/solvers/LexiVISolver.h"
#include "../../include/lexi/mobj_problem.h"
#include "../../include/lexi/mobj_state.h"
#include "../../include/util/general.h"

namespace mlsolvers
{

LexiVISolver::LexiVISolver(mlcore::Problem* problem, int maxIter, double tol)
{
    problem_ = problem;
    maxIter_ = maxIter;
    tol_ = tol;
}

mlcore::Action* LexiVISolver::solve(mlcore::State* s0)
{
    for (int i = 0; i < maxIter_; i++) {
        double maxResidual = 0.0;
        for (mlcore::State* s : problem_->states()) {
            double residual = lexiBellmanUpdate((mllexi::MOProblem *) problem_,
                                                 (mllexi::MOState *) s,
                                                 ((mllexi::MOProblem *) problem_)->size());
            if (residual > maxResidual)
                maxResidual = residual;
        }
        if (maxResidual < tol_)
            return nullptr;
    }
}

}
