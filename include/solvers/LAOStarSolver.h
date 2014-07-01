#ifndef MDPLIB_LAOSTARSOLVER_H
#define MDPLIB_LAOSTARSOLVER_H

namespace mlsolvers
{

    /**
     * A SSPP solver using the LAO* algorithm.
     *
     * See http://www.sciencedirect.com/science/article/pii/S0004370201001060
     */
    class LAOStarSolver
    {
    private:
        mlcore::Problem* problem_;
        mlcore::StateSet visited;

        /* Expands the BPSG rooted at state s and returns the number of states expanded */
        int expand(mlcore::State* s, int level);

        /* Test if the BPSG rooted at state s has converged */
        double testConvergence(mlcore::State* s, int level);
    public:
        /**
         * Creates a LAO* solver for the given problem.
         *
         * @param problem The problem to be solved.
         */
        LAOStarSolver(mlcore::Problem* problem) : problem_(problem) { }

        /**
         * Solves the associated problem using the LAO* algorithm.
         *
         * @param s0 The state to start the search at.
         * @param epsilon The error tolerance.
         */
        void solve(mlcore::State* s0, Rational epsilon);

    };
}

#endif // MDPLIB_LAOSTARSOLVER_H
