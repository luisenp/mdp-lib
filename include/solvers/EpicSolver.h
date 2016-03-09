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

    /*
     * Number of depth-limited expansions to do for each state.
     */
    int expansions_ = 1;

    /*
     * Number of trials to perform.
     */
    int trials_ = 1000;

    /*
     * Index used for the strongly connected component code.
     */
    int index_ = 0;

    /*
     * Store the states that have a non-zero probability of reaching a goal
     * using the greedy policy on the current values.
     */
    mlcore::StateSet statesCanReachGoal;

                                                                                mlcore::StateIntMap seenTrial;
                                                                                int trialIndex_;

    double
    computeProbabilityGoalMonteCarlo(mlcore::Problem* problem,
                                     mlcore::State* start);


    mlcore::StateSet visited_;

    void expandDepthLimited(mlcore::State* state, int depth);

    bool strongConnect(mlcore::State* state,
                       mlcore::StateIntMap& indices,
                       mlcore::StateIntMap& low,
                       std::list<mlcore::State*>& stateStack,
                       double& maxResidual);

public:
    EpicSolver(mlcore::Problem* problem,
               int horizon = 2,
               int expansions = 1,
               int trials = 1000) :
        problem_(problem), horizon_(horizon),
        expansions_(expansions),
        trials_(trials) { }

    virtual ~EpicSolver() { }

    virtual mlcore::Action* solve(mlcore::State* s0);

    /**
     * Returns true if the current greedy policy has a non-zero probability of
     * reaching the goal from the given state, and false otherwise.
     */
    bool canReachGoal(mlcore::State* state)
    {
        return (statesCanReachGoal.count(state) != 0);
    }

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
