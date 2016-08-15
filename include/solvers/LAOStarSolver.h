#ifndef MDPLIB_LAOSTARSOLVER_H
#define MDPLIB_LAOSTARSOLVER_H


#include <ctime>

#include "Solver.h"


namespace mlsolvers
{

/**
 * A SSPP solver using the LAO* algorithm.
 *
 * See http://www.sciencedirect.com/science/article/pii/S0004370201001060
 */
class LAOStarSolver : public Solver
{
private:
    mlcore::Problem* problem_;
    mlcore::StateSet visited;

    /* Error tolerance */
    double epsilon_ = 1.0e-6;

    /* Weight for the Bellman backup */
    double weight_ = 1.0;

    /* Time limit for LAO* in milliseconds */
    int timeLimit_ = 1000000;

    /*
     * Expands the BPSG rooted at state s and returns the
     * number of states expanded.
     */
    int expand(mlcore::State* s);

    /* Test if the BPSG rooted at state s has converged */
    double testConvergence(mlcore::State* s);

public:
    /**
     * Creates a LAO* solver for the given problem.
     *
     * @param problem The problem to be solved.
     * @param epsilon The error tolerance wanted for the solution.
     * @param timeLimit The maximum time allowed for running the algorithm.
     * @param weight The weight for the Bellman backup.
     */
    LAOStarSolver(mlcore::Problem* problem, double epsilon = 1.0e-6,
                  int timeLimit = 1000000, double weight = 1.0)
        : problem_(problem), epsilon_(epsilon),
          timeLimit_(timeLimit), weight_(weight) { }

    /**
     * Solves the associated problem using the LAO* algorithm.
     *
     * @param s0 The state to start the search at.
     */
    virtual mlcore::Action* solve(mlcore::State* s0);

};

}

#endif // MDPLIB_LAOSTARSOLVER_H
