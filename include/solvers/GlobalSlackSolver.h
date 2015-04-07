#ifndef MDPLIB_GLOBALSLACKSOLVER_H
#define MDPLIB_GLOBALSLACKSOLVER_H

#include "LAOStarSolver.h"

namespace mlsolvers
{

class GlobalSlackSolver : public Solver
{
private:

    mlmobj::MOProblem* problem_;

    /* The solver to use for each obj. func. evaluation */
    Solver* internalSolver_;

    /* Error tolerance */
    double epsilon_;

    /* Time limit for LAO* in milliseconds */
    int timeLimit_;

public:

    /**
     * Creates a GlobalSlackSolver for the given problem.
     *
     * @param problem The problem to be solved.
     * @param epsilon The error tolerance wanted for the solution.
     * @param timeLimit The maximum time allowed for running the algorithm.
     */
    GlobalSlackSolver(mlmobj::MOProblem* problem, double epsilon, int timeLimit)
        : problem_(problem), epsilon_(epsilon), timeLimit_(timeLimit)
    {
        internalSolver_ = new LAOStarSolver(problem);
    }

    virtual ~GlobalSlackSolver()
    {
        delete internalSolver_;
    }

    /**
     * Solves the associated problem using the GlobalSlackSolver algorithm.
     *
     * @param s0 The state to start the search at.
     */
    virtual mlcore::Action* solve(mlcore::State* s0);

};

}

#endif // MDPLIB_GLOBALSLACKSOLVER_H
