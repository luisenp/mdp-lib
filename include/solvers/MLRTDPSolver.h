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

    /* The error tolerance */
    double epsilon_;

    /* The max depth for the checkSolved procedure */
    int horizon_;

    /* If true the algorithm returns an optimal policy. */
    bool optimal_;

    /* Stores all states that have been solved to the latest horizon. */
    mlcore::StateSet depthSolved_;

    /* Performs a single trial */
    void trial(mlcore::State* s);

    /* Checks if the state has been solved. */
    bool checkSolved(mlcore::State* s);

    /* Checks if the state has already been labeled as solved. */
    bool labeledSolved(mlcore::State* s);

    /* Finds an optimal policy for the problem. */
    mlcore::Action* solveOptimally(mlcore::State* s0);

    /* Finds an approximate policy for the problem. */
    mlcore::Action* solveApproximate(mlcore::State* s0);

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
                 int horizon_,
                 bool optimal = false);

    /**
     * Solves the associated problem using the Labeled RTDP algorithm.
     *
     * @param s0 The state to start the search at.
     */
    virtual mlcore::Action* solve(mlcore::State* s0);

    void cleanup() { depthSolved_.clear(); }
};

}

#endif // MDPLIB_MLRTDPSOLVER_H
