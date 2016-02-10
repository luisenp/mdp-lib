#ifndef MDPLIB_LEXIVISOLVER_H
#define MDPLIB_LEXIVISOLVER_H

#include "../../mobj/MObjProblem.h"
#include "../../mobj/MObjState.h"

#include "../Solver.h"

namespace mdplib_mobj_solvers
{
/**
 * A LMDP solver that uses Lexicographic Value Iteration (LVI).
 * See http://anytime.cs.umass.edu/shlomo/papers/WZMaaai15.pdf
 */
class LexiVISolver : public mlsolvers::Solver
{

private:
    mlmobj::MOProblem* problem_;

    int maxIter_;

    double tol_;

public:
    /**
     * Creates a Lexicographical Value Iteration solver for the specified problem.
     *
     * @param problem The MOMDP problem to be solved.
     * @param maxIter The maximum number of iterations to perform.
     * @param tol The tolerance for the Bellman residual. If the residual is less
     *           than the tolerance the method stops.
     */
    LexiVISolver(mlmobj::MOProblem* problem, int maxIter = 100000, double tol = 1.0e-6);

    /**
     * Solves the associated problem using Lexicographical Value Iteration.
     *
     * Parameter s0 and return value only kept for compatibiliy with Solver
     * abstract class, they are not used by the method and the return value
     * is always a nullptr.
     *
     */
    virtual mlcore::Action* solve(mlcore::State* s0 = nullptr);
};

}

#endif // MDPLIB_LEXIVISOLVER_H
