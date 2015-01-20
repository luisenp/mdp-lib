#include <list>
#include <climits>
#include <cmath>

#include "../../include/lexi/domains/LexiRacetrackState.h"

#include "../../include/solvers/solver.h"
#include "../../include/solvers/LexiVISolver.h"
#include "../../include/lexi/lexi_problem.h"
#include "../../include/lexi/lexi_state.h"
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


//                mllexi::LexiRacetrackState* rts = (mllexi::LexiRacetrackState*) s;
//                if ((rts->x() == 31 && rts->y() == 29 && rts->vx() == 0 && rts->vy() == 0)
//                   || (rts->x() == 29 && rts->y() == 31 && rts->vx() == 3 && rts->vy() == -1))
//                    mdplib_debug = true;
//                else
//                    mdplib_debug = false;
//
//            dprint2(" ***** STATE ********** ", s);

            double residual = lexiBellmanUpdate((mllexi::LexiProblem *) problem_,
                                                 (mllexi::LexiState *) s);

//            dprint3(rts->lexiCost()[0]," ", rts->lexiCost()[1]);
//            dprint1(residual);

            if (residual > maxResidual)
                maxResidual = residual;
        }
        if (maxResidual < tol_)
            return nullptr;
    }
}

}
