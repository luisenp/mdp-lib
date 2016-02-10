#ifndef MDPLIB_MOLAOSTARSOLVER_H
#define MDPLIB_MOLAOSTARSOLVER_H

#include "../Solver.h"

#include "../../mobj/MObjProblem.h"
#include "../../mobj/MObjState.h"

namespace mdplib_mobj_solvers
{

class MOLAOStarSolver : public mlsolvers::Solver
{
private:
    mlmobj::MOProblem* problem_;
    mlcore::StateSet visited_;
    mlcore::StateSet solved_;
    mlcore::StateSet initialized_;

    /* Error tolerance */
    double epsilon_;

    /*
     * Expands the BPSG rooted at state s and returns the number of states
     * The choice of action is done according to the specified lexicographic
     * level.
     * If a state is found that wasn't solved at the previous level,
     * the state is returned.
     */
    int expand(mlmobj::MOState* s, int level, mlmobj::MOState*& unsolved);

    /*
     * Test if the BPSG rooted at state s has converged at the given
     * lexicographic level.
     */
    double testConvergence(mlmobj::MOState* s, int level);

    /*
     * Adds all states for which the optimal policy starting in state s
     * has been found.
     */
    void addSolved(mlcore::State* s);

    /* Time limit for LAO* in milliseconds */
    int timeLimit_;

    /* Weights for combining the cost functions */
    std::vector<double> weights_;

    /*
     * If set to true, it uses a linear combination of the cost functions
     * for search
     */
    bool useLC_;

    /*
     * Solves the specified lexicographic level. The stores a pointer to a state
     * only if this state was unsolved at the previous level.
     */
    virtual void
    solveLevel(mlcore::State* s0, int level, mlmobj::MOState*& unsolved);

public:

    MOLAOStarSolver();

    virtual ~MOLAOStarSolver() {}

    /**
     * Creates a Lexicographical LAO* solver for the given problem.
     *
     * @param problem The problem to be solved.
     * @param epsilon The error tolerance wanted for the solution.
     * @param timeLimit The maximum time allowed for running the algorithm.
     * @param useLC true if the cost functions should be combined linearly.
     */
    MOLAOStarSolver(mlmobj::MOProblem* problem, double epsilon = 1.0e-6,
                    int timeLimit = 1000000, bool useLC = false)
        : problem_(problem), epsilon_(epsilon),
          timeLimit_(timeLimit), useLC_(useLC)
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


} // namespace mdplib_mobj_solvers

#endif // MDPLIB_MOLAOSTARSOLVER_H
