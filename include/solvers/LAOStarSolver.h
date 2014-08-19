#ifndef MDPLIB_LAOSTARSOLVER_H
#define MDPLIB_LAOSTARSOLVER_H

namespace mlsolvers
{

    /**
     * A SSPP solver using the LAO* algorithm.
     *
     * See http://www.sciencedirect.com/science/article/pii/S0004370201001060
     */
    class LAOStarSolver : public Solver
    {
    private:
        mlcore::Problem* problem_;
        mlcore::StateSet visited;

        double epsilon_;

        /* Expands the BPSG rooted at state s and returns the number of states expanded */
        int expand(mlcore::State* s);

        /* Test if the BPSG rooted at state s has converged */
        double testConvergence(mlcore::State* s);

        /* Time limit for LAO* in milliseconds */
        int timeLimit_;

    public:
        /**
         * Creates a LAO* solver for the given problem.
         *
         * @param problem The problem to be solved.
         * @param epsilon The error tolerance wanted for the solution.
         */
        LAOStarSolver(mlcore::Problem* problem, double epsilon, int timeLimit)
            : problem_(problem), epsilon_(epsilon), timeLimit_(timeLimit) { }

        /**
         * Solves the associated problem using the LAO* algorithm.
         *
         * @param s0 The state to start the search at.
         */
        virtual mlcore::Action* solve(mlcore::State* s0);

    };
}

#endif // MDPLIB_LAOSTARSOLVER_H
