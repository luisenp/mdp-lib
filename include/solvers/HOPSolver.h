#ifndef MDPLIB_HOPSOLVER_H
#define MDPLIB_HOPSOLVER_H


#include "../Problem.h"
#include "../Heuristic.h"

#include "Solver.h"

namespace mlsolvers
{

/**
 * An SSP solver that uses Hindsight Optimization.
 * See https://engineering.purdue.edu/~givan/papers/aaai08-sy.pdf
 */
class HOPSolver : public Solver
{
private:

    mlcore::Problem* problem_;

    /* The maximum number of futures to sample. */
    int maxSamples_;

    /* Look-ahead Horizon for the futures. */
    int horizon_;

    /* Maximum planning time in milliseconds. */
    int maxTime_;

    /* The time at which planning began. */
    std::chrono::time_point<std::chrono::high_resolution_clock> beginTime_;

    /* Returns true iff there is no more time left for planning. */
    bool ranOutOfTime();

public:
    /**
     * Creates a HOPSolver solver for the given problem.
     *
     * @param problem The problem to be solved.
     * @param maxSamples The maximum number of futures to sample.
     * @param horizon The look-ahead horizon for the futures.
     * @param maxTime The maximum time allowed for planning.
     */
    HOPSolver(mlcore::Problem* problem,
              int maxSamples = 100,
              int horizon = 5,
              int maxTime = -1);

    /**
     * Solves the associated problem using Hindsight Optimization.
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

#endif // MDPLIB_HOPSOLVER_H
