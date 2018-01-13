#ifndef MDPLIB_SSIPPSOLVER_H
#define MDPLIB_SSIPPSOLVER_H


#include "../domains/WrapperProblem.h"

#include "Solver.h"


namespace mlsolvers
{

enum class SSiPPAlgo {
    Original,
    Labeled
};

/**
 * An implementation of the Short-Sighted Probabilistic Planner
 * described in (AI'14).
 *
 * http://felipe.trevizan.org/papers/trevizan14:depth.pdf
 */
class SSiPPSolver : public Solver
{
private:
    /* The problem to solve. */
    mlcore::Problem* problem_;

    /* The error tolerance. */
    double epsilon_;

    /* The maximum number of trials allowed to the optimal solver. */
    int maxTrials_;

    /* The horizon for the Short-sighted SSP. */
    int t_;

    /* The SSiPP algorithm version to use. */
    SSiPPAlgo algorithm_;

    /*
     * Solves using the original depth-based SSiPP solver from ICAPS'12.
     * http://www.cs.cmu.edu/~mmv/papers/12icaps-TrevizanVeloso.pdf
     */
    mlcore::Action* solveOriginal(mlcore::State* s0);

    /*
     * Solves using the labeled version from AI'14.
     * http://felipe.trevizan.org/papers/trevizan14:depth.pdf
     */
    mlcore::Action* solveLabeled(mlcore::State* s0);

    /* A procedure that checks for solved states and labels them. */
    bool checkSolved(mlcore::State* s);

    /* An optimal solver to use for the short-sighted SSPs. */
    void optimalSolver(WrapperProblem* problem, mlcore::State* s0);

public:
    /**
     * Creates a new SSiPP solver for the given problem. The constructor
     * specifies the error tolerance (epsilon) and the horizon for the
     * short-sighted SSPs (t).
     */
    SSiPPSolver(mlcore::Problem* problem,
                double epsilon = 1.0e-6,
                int t = 3,
                SSiPPAlgo algorithm = SSiPPAlgo::Original) :
        problem_(problem),
        epsilon_(epsilon),
        t_(t),
        algorithm_(algorithm),
        maxTrials_(10000000) { }

    virtual ~SSiPPSolver() { }

    /**
     * Solves the associated problem using the Labeled RTDP algorithm.
     *
     * @param s0 The state to start the search at.
     */
    virtual mlcore::Action* solve(mlcore::State* s0);

    /**
     * Sets the maximum number of trials allowed to the algorithm.
     */
    virtual void maxTrials(time_t theTrials) { maxTrials_  = theTrials; }
};

}  // namespace mlsolvers

#endif // MDPLIB_SSIPPSOLVER_H
