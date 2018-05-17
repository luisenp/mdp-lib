#ifndef MDPLIB_LRTDPSOLVER_H
#define MDPLIB_LRTDPSOLVER_H

#include "../Problem.h"
#include "../Heuristic.h"

#include "Solver.h"

namespace mlsolvers
{

/**
 * An SSP solver using the Labeled RTDP algorithm.
 *
 * See http://www.aaai.org/Papers/ICAPS/2003/ICAPS03-002.pdf
 */
class LRTDPSolver : public Solver
{
private:

    mlcore::Problem* problem_;
    int maxTrials_;
    double epsilon_;

    /* Maximum planning time in milliseconds. */
    int maxTime_;

    /* If true the algorithm runs like RTDP (no labeling). */
    bool dont_label_;

    /* Performs a single LRTDP trial */
    void trial(mlcore::State* s);

    /* Checks if the state has been solved. */
    bool checkSolved(mlcore::State* s);

    /* The time at which planning began. */
    std::chrono::time_point<std::chrono::high_resolution_clock> beginTime_;

    /* Returns true iff there is no more time left for planning. */
    bool ranOutOfTime();

public:
    /**
     * Creates a LRTDP solver for the given problem.
     *
     * @param problem The problem to be solved.
     * @param maxTrials The maximum number of trials to perform.
     * @param epsilon The error tolerance.
     * @param maxTime The maximum time allowed for planning.
     * @param dont_label If true, no labeling will be used (runs like RTDP).
     */
    LRTDPSolver(mlcore::Problem* problem,
                int maxTrials,
                double epsilon,
                int maxTime = -1,
                bool dont_label = false);

    /**
     * Solves the associated problem using the Labeled RTDP algorithm.
     *
     * @param s0 The state to start the search at.
     */
    virtual mlcore::Action* solve(mlcore::State* s0);

    /**
     * Sets the maximum planning time allowed to the algorithm (milliseconds).
     */
    virtual void maxPlanningTime(time_t theTime) { maxTime_ = theTime; }

};

}

#endif // MDPLIB_LRTDPSOLVER_H
