#ifndef MDPLIB_EPICSOLVER_H
#define MDPLIB_EPICSOLVER_H


#include "../Problem.h"
#include "../State.h"

#include "../domains/WrapperProblem.h"

#include "Solver.h"

namespace mlsolvers
{

class EpicSolver : public Solver
{
private:
    mlcore::Problem* problem_;

    /*
     * Runs a trial of the greedy policy starting in state.
     */
    void trial(mlcore::State* start);

    /*
     * Performs depth-limited Value Iteration on the given state.
     * The WrapperProblem is used to add arbitrary sets of goals
     */
    void solveDepthLimited(mlcore::State* state, WrapperProblem* wrapper);

    /*
     * Depth for the depth-limited Value Iteration.
     */
    int horizon_ = 3;

    double
    computeProbabilityGoalMonteCarlo(mlcore::Problem* problem,
                                     mlcore::State* start);


    mlcore::StateSet visited_;

    void expandDepthLimited(mlcore::State* state, int depth);

public:
    EpicSolver(mlcore::Problem* problem, int horizon) :
        problem_(problem), horizon_(horizon) { }

    virtual ~EpicSolver() { }

    virtual mlcore::Action* solve(mlcore::State* s0);

    /**
     * Computes the probability of reaching states outside a set of
     * envelope states, starting at the given initial state, using
     * the greedy policy for the states in the problem.
     *
     * @param start The initial state.
     * @param envelope A set of states.
     * @return A map from state to doubles storing the computed probabilities.
     */
    static mlcore::StateDoubleMap
    computeProbabilityTerminals(mlcore::Problem* problem,
                                mlcore::State* start,
                                mlcore::StateSet& envelope);

};

}

#endif // MDPLIB_EPICSOLVER_H
