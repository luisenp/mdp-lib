#ifndef MDPLIB_LEXILAOSTARSOLVER_H
#define MDPLIB_LEXILAOSTARSOLVER_H

#include "../lexi/lexi_problem.h"

namespace mlsolvers
{

class LexiLAOStarSolver : public Solver
{
private:
    mllexi::LexiProblem* problem_;
    mlcore::StateSet visited;

    /* Error tolerance */
    double epsilon_;

    /* Expands the BPSG rooted at state s and returns the number of states expanded */
    int expand(mllexi::LexiState* s);

    /* Test if the BPSG rooted at state s has converged */
    double testConvergence(mllexi::LexiState* s);

    /* Time limit for LAO* in milliseconds */
    int timeLimit_;

public:
    LexiLAOStarSolver();

    virtual ~LexiLAOStarSolver() {}

    /**
     * Creates a Lexicographical LAO* solver for the given problem.
     *
     * @param problem The problem to be solved.
     * @param epsilon The error tolerance wanted for the solution.
     * @param timeLimit The maximum time allowed for running the algorithm.
     */
    LexiLAOStarSolver(mllexi::LexiProblem* problem, double epsilon, int timeLimit)
        : problem_(problem), epsilon_(epsilon), timeLimit_(timeLimit) { }

    /**
     * Solves the associated problem using the Lexicographic LAO* algorithm.
     *
     * @param s0 The state to start the search at.
     */
    virtual mlcore::Action* solve(mlcore::State* s0);
};


}

#endif // MDPLIB_LEXILAOSTARSOLVER_H
