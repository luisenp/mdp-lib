#include <list>
#include <climits>
#include <cmath>

#include "../../../include/mobj/domains/MORacetrackState.h"

#include "../../../include/solvers/solver.h"
#include "../../../include/solvers/mobj/LexiVISolver.h"
#include "../../../include/mobj/mobj_problem.h"
#include "../../../include/mobj/mobj_state.h"
#include "../../../include/util/general.h"

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
            double residual = lexiBellmanUpdate((mlmobj::MOProblem *) problem_,
                                                 (mlmobj::MOState *) s,
                                                 ((mlmobj::MOProblem *) problem_)->size());
            if (residual > maxResidual)
                maxResidual = residual;
        }
        if (maxResidual < tol_)
            return nullptr;
    }
}

}
