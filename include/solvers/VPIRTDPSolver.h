#ifndef MDPLIB_VPIRTDP_H
#define MDPLIB_VPIRTDP_H

#include "Solver.h"

#include "../State.h"

namespace mlsolvers {

/**
 * An SSP solver using the VPI-RTDP algorithm.
 * https://www.aaai.org/ocs/index.php/IJCAI/IJCAI-09/paper/viewFile/552/879
 */
class VPIRTDPSolver : public Solver
{
private:

    mlcore::Problem* problem_;

    /* The maximum number of trials to perform. */
    int maxTrials_;

    /* Criterion for stopping the algorithm
      (the difference between the bounds). */
    double epsilon_;

    /* Criterion for considering a state as well-known. */
    double tau_;

    /* If the average successor bound gap is larger than |beta_|, then
     * VPI will not be used (not enough information). */
    double beta_;

    /* The probability that we return a successor based solely on the bounds
     * when VPI of all outcomes is 0. */
    double alpha_;

    /* Upper bounds on the state costs. */
    mlcore::StateDoubleMap upperBounds_;

    mlcore::StateActionMap lowerBoundGreedyPolicy_;

    /* Performs a single BRTDP trial. */
    void trial(mlcore::State* s);

    /* Performs a Bellman update for the state, changing its estimated cost
     * and the values of the bounds. */
    double bellmanUpdate(mlcore::State* s);

    /* Initializes the upper bound for the given state. */
    void initializeUpperBound(mlcore::State* s);

    /*
     * Samples a state biased according to the difference of its bounds.
     * Returns a nullptr if all successor states have "well-known" value.
     * The output parameter |B| stores the average bound gap over the
     * set of successors.
     */
    mlcore::State* sampleBiasedBounds(mlcore::State* s,
                                      mlcore::Action* sampledAction,
                                      double& B);

    /*
     * Samples a state according to a myopic VPI analysis.
     */
    mlcore::State* sampleVPI(mlcore::State* s, mlcore::Action* sampledAction);

public:
    /**
     * Creates a BRTDP solver for the given problem.
     *
     * @param problem The problem to be solved.
     * @param maxTrials The maximum number of trials to perform.
     * @param epsilon The error tolerance.
     */
    VPIRTDPSolver(mlcore::Problem* problem,
                      double epsilon,
                      int maxTrials = 1000000);

    /**
     * Solves the associated problem using the Labeled RTDP algorithm.
     *
     * @param s0 The state to start the search at.
     */
    virtual mlcore::Action* solve(mlcore::State* s0);
};

}

#endif // MDPLIB_VPIRTDP_H
