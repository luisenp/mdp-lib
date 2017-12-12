#ifndef MDPLIB_SoftFLARESSolver_H
#define MDPLIB_SoftFLARESSolver_H

#include "Solver.h"

#include "../Heuristic.h"
#include "../Problem.h"
#include "../State.h"


namespace mlsolvers
{

enum TransitionModifierFunction {
    kExponentialDecay,
    kLogistic,
    kStep,
};

/**
 * A SSP solver using the Soft-FLARES algorithm.
 */
class SoftFLARESSolver : public Solver {
private:
    /* ********************************************************************* *
                                    Variables
    /* ********************************************************************* */
    /* The problem to solve. */
    mlcore::Problem* problem_;

    /* The maximum number of trials. */
    int maxTrials_;

    /* If optimal is true, the algorithm will be run until the initial state
     * is marked as solved. */
    bool optimal_;

    /* The error tolerance */
    double epsilon_;

    /* The max depth for the checkSolved procedure */
    double horizon_;

    /*
     * Represents the desired reduction in the transition function at the
     * horizon. For example, if |alpha| = 0.01, then a state s' that has
     * |horizon_| steps of successors with residual < |epsilon_| will
     * have a score of 0.01 * T(s,a,s').
     */
    double alpha_;

    /*
     * Modifies the scoring function to achieve the desired |alpha_|.
     */
    double tau_;

    /*
     * If true, the depth of states will be the log probability of reaching
     * the state. Otherwise, it is the number of steps.
     */
    bool useProbsForDepth_;

    /*
     * The function used to modify the transition function from the residual
     * distance estimates.
     */
    TransitionModifierFunction modifierFunction_;

    /*
     * Stores an upper bound of how many steps in the greedy policy a state
     * is from a state with low residual error.
     */
    mlcore::StateDoubleMap lowResidualDistance_;

    /* Stores the result of modifier function for depths from 0 to horizon_*/
    std::vector<double> modifierCache_;

    /* ********************************************************************* *
                                    Methods
    /* ********************************************************************* */
    /*
     * Samples a successor biased towards state closer to higher residual
     * error.
     */
    mlcore::State* sampleSuccessor(mlcore::State* s, mlcore::Action* a);

    /* Computes the distances to states with high residuals. */
    void computeResidualDistances(mlcore::State* s);

    /* Performs a single trial */
    void trial(mlcore::State* s);

    /* Checks if the state has already been labeled as solved. */
    bool labeledSolved(mlcore::State* s);

    /*
     * Computes the depth of the given successor state for the given depth of
     * its parent state.
     */
    double computeNewDepth(mlcore::Successor& su, double depth);

    /*
     * Computes the modifier from which the new sampling distribution is
     * generated. The sampling probability will be
     *   T'(s,a,s') prop. T(s,a,s') * modifier(|lowResidualDistance_|(s')).
     * The type of modification is controlled using |modifierFunction_|.
     */
    double computeProbModfier(mlcore::State* s);
/*
     * Computes the modifier from which the new sampling distribution is
     * generated, given a distance. The sampling probability will be
     *   T'(s,a,s') prop. T(s,a,s') * modifier(distance).
     * The type of modification is controlled using |modifierFunction_|.
     */
    double computeProbModfier(double distance);

    /*
     * Computes the unnormalized transition function using an exponential
     * decaying modifier and stores the result in the |scores| vector.
     * |totalScore| is the sum of all the scores, needed to normalize the new
     * distribution.
     */
    mlcore::State* computeScores(mlcore::State* s,
                                 mlcore::Action* a,
                                 std::vector<double>& scores,
                                 double& totalScore);

                                                                                int cnt_samples_ = 0;
                                                                                long int total_time_samples_ = 0;
                                                                                int cnt_check_ = 0;
                                                                                long int total_time_check_ = 0;

public:
    /**
     * Creates a FLARES solver for the given problem.
     *
     * @param problem The problem to be solved.
     * @param maxTrials The maximum number of trials to perform.
     * @param epsilon The error tolerance.
     * @param horizon_ The maximum depth for checkSolved.
     * @param modifierFunction The function to use to modify the transition.
     * @param alpha Desired vanishing level.
     * @param useProbsForDepth If true, uses trajectory probabilities as
     *        the depth (instead of the number of steps).
     * @param optimal If true, runs until optimality.
     */
    SoftFLARESSolver(mlcore::Problem* problem,
                     int maxTrials,
                     double epsilon,
                     double horizon_,
                     TransitionModifierFunction modifierFunction,
                     double alpha = 0.01,
                     bool useProbsForDepth = false,
                     bool optimal = false);

    /**
     * Solves the associated problem using the Labeled RTDP algorithm.
     *
     * @param s0 The state to start the search at.
     */
    virtual mlcore::Action* solve(mlcore::State* s0);

    double lowResidualDistance( mlcore::State* s) const {
        if (lowResidualDistance_.count(s) > 0)
            return lowResidualDistance_.at(s);
        // A distance of -1 indicates high residual error for this state
        // (note that distance means that the state value has low residual error
        // but its successor don't)
        return -1;
    }

    double horizon() const { return horizon_; }
};

}

#endif // MDPLIB_SoftFLARESSolver_H
