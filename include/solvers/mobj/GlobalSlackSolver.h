#ifndef MDPLIB_GLOBALSLACKSOLVER_H
#define MDPLIB_GLOBALSLACKSOLVER_H

#include "../Solver.h"

#include "MOLAOStarSolver.h"

namespace mdplib_mobj_solvers
{

class GlobalSlackSolver : public mlsolvers::Solver
{
private:

    mlmobj::MOProblem* problem_;

    /* The solver to use for each obj. func. evaluation */
    Solver* internalSolver_;

    /* Error tolerance */
    double epsilon_;

    /* Time limit for LAO* in milliseconds */
    int timeLimit_;

    /* Reset values for all states */
    void resetStates(std::vector<double>& weights)
    {
        for (mlcore::State* x : problem_->states()) {
            ((mlmobj::MOState * ) x)->resetCost(weights, -1);
            ((mlmobj::MOState * ) x)->setBestAction(nullptr);
        }
    }

    /* Temporary method (TODO: delete eventually if it doesn't work) */
    mlcore::Action* binarySearch(mlcore::State* s);

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
        internalSolver_ = new MOLAOStarSolver(problem, epsilon, timeLimit, true);
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

} // namespace mdplib_mobj_solvers

#endif // MDPLIB_GLOBALSLACKSOLVER_H
