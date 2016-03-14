#ifndef MDPLIB_MLRTDPSOLVER_H
#define MDPLIB_MLRTDPSOLVER_H

#include "../Problem.h"
#include "../Heuristic.h"

#include "Solver.h"

namespace mlsolvers
{
    /**
     * A SSP solver using the Myopic LRTDP algorithm.
     */
    class MLRTDPSolver : public Solver
    {
    private:
        /* The problem to solve. */
        mlcore::Problem* problem_;

        /* The maximum number of trials. */
        int maxTrials_;


        double epsilon_;

        /* Performs a single trial */
        void trial(mlcore::State* s);

        /* Checks if the state has been solved. */
        bool checkSolved(mlcore::State* s);

        /* The max depth for the checkSolved procedure */
        int horizon_;

    public:
        /**
         * Creates a Myopic LRTDP solver for the given problem.
         *
         * @param problem The problem to be solved.
         * @param maxTrials The maximum number of trials to perform.
         * @param epsilon The error tolerance.
         * @param horizon_ The maximum depth for checkSolved.
         */
        MLRTDPSolver(mlcore::Problem* problem,
                     int maxTrials,
                     double epsilon,
                     int horizon_);

        /**
         * Solves the associated problem using the Labeled RTDP algorithm.
         *
         * @param s0 The state to start the search at.
         */
        virtual mlcore::Action* solve(mlcore::State* s0);
    };
}

#endif // MDPLIB_MLRTDPSOLVER_H
