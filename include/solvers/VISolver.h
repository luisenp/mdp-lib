#ifndef MDPLIB_VISOLVER_H
#define MDPLIB_VISOLVER_H

#include "../problem.h"

namespace mlsolvers
{
    /**
     * A MDP solver that using Value Iteration.
     */
    class VISolver
    {
    private:
        mlcore::Problem* problem_;

    public:
        /**
         * Creates a Value Iteration solver for the specified problem.
         *
         * @param problem The problem to be solved.
         */
        VISolver(mlcore::Problem* problem);

        /**
         * Solves the associated problem using Value Iteration.
         *
         * @param maxIter The maximum number of iterations to perform.
         * @param tol The tolerance for the Bellman residual. If the residual is less
         *           than the tolerance the method stops.
         */
        void solve(int maxIter, Rational tol);
    };
}

#endif // MDPLIB_VISOLVER_H
