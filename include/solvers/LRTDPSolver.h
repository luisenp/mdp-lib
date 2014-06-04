#ifndef MDPLIB_LRTDPSOLVER_H
#define MDPLIB_LRTDPSOLVER_H

#include "../problem.h"
#include "../heuristic.h"

/**
 * A SSPP solver using the Labeled RTDP algorithm.
 *
 * See http://www.aaai.org/Papers/ICAPS/2003/ICAPS03-002.pdf
 */
class LRTDPSolver
{
private:
    Problem* problem_;

    /* Performs a single LRTDP trial */
    void trial(Rational epsilon);

    /* Checks if the state has been solved */
    bool checkSolved(State* s, Rational epsilon);

public:
    /**
     * Creates a LRTDP solver for the given problem.
     *
     * @param problem The problem to be solved.
     */
    LRTDPSolver(Problem* problem);

    /**
     * Solves the associated problem using the Labeled RTDP algorithm.
     *
     * @param s0 The state to start the search at.
     * @param maxTrials The maximum number of trials to perform.
     * @param epsilon The error tolerance.
     */
    void solve(State* s0, int maxTrials, Rational epsilon);

};

#endif // MDPLIB_LRTDPSOLVER_H
