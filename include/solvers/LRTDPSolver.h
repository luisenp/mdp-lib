#ifndef MDPLIB_LRTDPSOLVER_H
#define MDPLIB_LRTDPSOLVER_H

#include "../problem.h"
#include "../heuristic.h"

/**
 * A SSPP solver using the Labeled RTDP algorithm.
 *
 * See ftp://webarchive.cs.ucla.edu/pub/stat_ser/R319.pdf
 */
class LRTDPSolver
{
private:
    Problem* problem_;
    Heuristic* heuristic_;

    StateSet expanded;  /* States that have been expanded so far */

    void trial();   /* Performs a single LRTDP trial */

public:
    /**
     * Creates a LRTDP solver for the given problem using given heuristic.
     *
     * @param problem The problem to be solved.
     * @param heuristic The heuristic to use.
     */
    LRTDPSolver(Problem* problem, Heuristic* heuristic);

    /**
     * Solves the associated problem using the Labeled RTDP algorithm.
     *
     * @param maxTrials The maximum number of trials to perform.
     */
    void solve(int maxTrials);
};

#endif // MDPLIB_LRTDPSOLVER_H
