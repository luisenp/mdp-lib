#ifndef MDPLIB_WLAO_H
#define MDPLIB_WLAO_H

#include "solver.h"


namespace mlsolvers
{

class WeightedLAOStarSolver : public Solver
{
private:
    mlcore::Problem* problem_;
    mlcore::StateSet visited;

    double epsilon_;

    /* Expands the BPSG rooted at state s and returns the number of states expanded */
    int expand(mlcore::State* s);

    /* Test if the BPSG rooted at state s has converged */
    double testConvergence(mlcore::State* s);

    /* Time limit for LAO* in milliseconds */
    int timeLimit_;

    mlcore::StateDoubleMap hValues_;
    mlcore::StateDoubleMap gValues_;
    mlcore::StateActionMap bestActions_;

    double weight_;

    double bellmanUpdate(mlcore::State* s);

public:

    /**
     * Creates a weighted-LAO* solver for the given problem.
     *
     * @param problem The problem to be solved.
     * @param weight The weight to use.
     * @param epsilon The error tolerance wanted for the solution.
     * @param timeLimit The maximum time allowed for running the algorithm.
     */
    WeightedLAOStarSolver(mlcore::Problem* problem, double weight, double epsilon, int timeLimit)
        : problem_(problem), weight_(weight), epsilon_(epsilon), timeLimit_(timeLimit) { }

    virtual ~WeightedLAOStarSolver() { }

    /**
     * Solves the associated problem using the LAO* algorithm.
     *
     * @param s0 The state to start the search at.
     */
    virtual mlcore::Action* solve(mlcore::State* s0);
};


}


#endif // MDPLIB_WLAO_H
