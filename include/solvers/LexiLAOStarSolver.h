#ifndef MDPLIB_LEXILAOSTARSOLVER_H
#define MDPLIB_LEXILAOSTARSOLVER_H

#include "../lexi/mobj_problem.h"

namespace mlsolvers
{

class LexiLAOStarSolver : public Solver
{
private:
    mllexi::MOProblem* problem_;
    mlcore::StateSet visited_;
    mlcore::StateSet solved_;
    mlcore::StateSet initialized_;

    /* Error tolerance */
    double epsilon_;

    /* Expands the BPSG rooted at state s and returns the number of states
     * The choice of action is done according to the specified lexicographic level
     * If a state is found that wasn't solve at the previous level, the state is returned */
    int expand(mllexi::MOState* s, int level, mllexi::MOState*& unsolved);

    /* Test if the BPSG rooted at state s has converged at the given lexicographic level*/
    double testConvergence(mllexi::MOState* s, int level);

    /* Adds all states for which the optimal policy starting in state s has been found */
    void addSolved(mlcore::State* s);

    /* Time limit for LAO* in milliseconds */
    int timeLimit_;

    /* Weights for combining the cost functions */
    std::vector<double> weights_;

    /* Solves the specified lexicographic level. The stores a pointer to a state
    /* only if this state was unsolved at the previous level. */
    virtual void solveLevel(mlcore::State* s0, int level, mllexi::MOState*& unsolved);

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
    LexiLAOStarSolver(mllexi::MOProblem* problem, double epsilon, int timeLimit)
        : problem_(problem), epsilon_(epsilon), timeLimit_(timeLimit)
    {
        weights_ = std::vector<double> (problem->size(), 0.0);
        weights_[0] = 1.0;
    }

    /**
     * Solves the associated problem using the Lexicographic LAO* algorithm.
     *
     * @param s0 The state to start the search at.
     */
    virtual mlcore::Action* solve(mlcore::State* s0);

    void weights(std::vector<double> weights)  { weights_ = weights; }
};


}

#endif // MDPLIB_LEXILAOSTARSOLVER_H
