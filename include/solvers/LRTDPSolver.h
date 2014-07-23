#ifndef MDPLIB_LRTDPSOLVER_H
#define MDPLIB_LRTDPSOLVER_H

#include "solver.h"

#include "../problem.h"
#include "../heuristic.h"

namespace mlsolvers
{
    /**
     * A SSPP solver using the Labeled RTDP algorithm.
     *
     * See http://www.aaai.org/Papers/ICAPS/2003/ICAPS03-002.pdf
     */
    class LRTDPSolver : public Solver
    {
    private:

        mlcore::Problem* problem_;
        int maxTrials_;
        double epsilon_;

        /* The maximum number of states that are visited on a call to checkSolved */
        int maxChecked_ = 1000000;

        /* Performs a single LRTDP trial */
        void trial(mlcore::State* s);

        /* Checks if the state has been solved */
        bool checkSolved(mlcore::State* s);

    public:
        /**
         * Creates a LRTDP solver for the given problem.
         *
         * @param problem The problem to be solved.
         * @param maxTrials The maximum number of trials to perform.
         * @param epsilon The error tolerance.
         */
        LRTDPSolver(mlcore::Problem* problem, int maxTrials, double epsilon);

        /**
         * Solves the associated problem using the Labeled RTDP algorithm.
         *
         * @param s0 The state to start the search at.
         */
        virtual mlcore::Action* solve(mlcore::State* s0);

        /**
        * Sets the maximum number of states that are visited on a call to checkSolved.
        * Usually there is no upper bound on this quantity. However, in some problems
        * the set of states that can be visited can be quite large. Since no backups
        * are performed in checkSolved until all reachable states are visited, this
        * impairs the performance of LRTDP in online settings.
        *
        * param maxChecked the maximum number of states that are visited on a call
        * to checkSolved.
        */
        void setMaxChecked(int maxChecked) { maxChecked_ = maxChecked; }

    };
}

#endif // MDPLIB_LRTDPSOLVER_H
