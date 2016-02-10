#ifndef MDPLIB_VISOLVER_H
#define MDPLIB_VISOLVER_H

#include "../Problem.h"
#include "../State.h"

#include "Solver.h"

namespace mlsolvers
{
    /**
     * A MDP solver that using Value Iteration.
     */
    class VISolver : public Solver
    {
    private:
        mlcore::Problem* problem_;

        int maxIter_;

        double tol_;

    public:
        /**
         * Creates a Value Iteration solver for the specified problem.
         *
         * @param problem The problem to be solved.
         * @param maxIter The maximum number of iterations to perform.
         * @param tol The tolerance for the Bellman residual.
         *            If the residual is less than the tolerance the
         *            method stops.
         */
        VISolver(mlcore::Problem* problem,
                 int maxIter = 100000,
                 double tol = 1.0e-6);

        /**
         * Solves the associated problem using Value Iteration.
         *
         * Parameter s0 and return value only kept for compatibility with Solver
         * abstract class, they are not used by the method and the return value
         * is always a nullptr.
         *
         */
        virtual mlcore::Action* solve(mlcore::State* s0 = nullptr);
    };
}

#endif // MDPLIB_VISOLVER_H
