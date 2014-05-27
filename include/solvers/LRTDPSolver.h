#ifndef MDPLIB_LRTDPSOLVER_H
#define MDPLIB_LRTDPSOLVER_H

#include "../problem.h"

/**
 * A SSPP solver using the Labeled RTDP algorithm.
 *
 * See ftp://webarchive.cs.ucla.edu/pub/stat_ser/R319.pdf
 */
class LRTDPSolver
{
private:
    Problem* problem_;
public:
    /**
     * Creates a LRTDP solver for the specified problem.
     *
     * @param problem The problem to be solved.
     */
    LRTDPSolver(Problem* problem);

    /**
     * Solves the associated problem using the Labeled RTDP algorithm.
     *
     * @param maxTrials The maximum number of trials to perform.
     */
    void solve(int maxTrials);
};

#endif // MDPLIB_LRTDPSOLVER_H
