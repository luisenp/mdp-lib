#ifndef MDPLIB_SOLVER_H
#define MDPLIB_SOLVER_H

#include <random>
#include <cassert>
#include <vector>
#include <mutex>

#include "../Heuristic.h"
#include "../Problem.h"
#include "../State.h"
#include "../util/general.h"

#define bb_cost first
#define bb_action second

namespace mlsolvers
{
/**
 * A mutex used by BellmanUpdate in order to avoid race conditions while
 * planning/executing concurrently.
 */
extern std::mutex bellman_mutex;

/**
 * Random number generator.
 */
extern std::random_device rand_dev;

/**
 * Mersenne Twister 19937 generator.
 */
extern std::mt19937 gen;

/**
 * Uniform distribution [0,1] generator.
 */
extern std::uniform_real_distribution<> dis;

/**
 * An interface describing planning algorithms.
 */
class Solver
{
public:

    /**
     * Solves the associated problem using this solver.
     *
     * @param s0 The state to start the search at.
     * @return The action recommended by this algorithm for state s0.
     */
    virtual mlcore::Action* solve(mlcore::State* s0) =0;
};

/**
 * Computes the Q-value of a state-action pair.
 * This method assumes that the given action is applicable on the state.
 *
 * @param problem The problem that contains the given state.
 * @param s The state for which the Q-value will be computed.
 * @param a The action for which the Q-value will be computed.
 * @return The Q-value of the state-action pair.
 */
double qvalue(mlcore::Problem* problem, mlcore::State* s, mlcore::Action* a);


/**
 * Computes the weighted-Q-value of a state-action pair.
 * This method assumes that the given action is applicable on the state.
 *
 * @param problem The problem that contains the given state.
 * @param s The state for which the Q-value will be computed.
 * @param a The action for which the Q-value will be computed.
 * @param weight The weight to use.
 * @return A pair of doubles representing the g-value and h-value of the
 *         state-action pair.
  *        The weighted-Q-value can be recovered as g + weight * h.
 */
std::pair<double, double>
weightedQvalue(mlcore::Problem* problem, mlcore::State* s, mlcore::Action* a);


/**
 * Performs a Bellman backup of a state.
 *
 * This backup uses fSSPUDE - see http://arxiv.org/pdf/1210.4875.pdf
 *
 * @param problem The problem that contains the given state.
 * @param s The state on which the Bellman backup will be performed.
 * @return A pair containing the estimated cost and estimated best action
 *         according to this Bellman backup.
 */
std::pair<double, mlcore::Action*>
bellmanBackup(mlcore::Problem* problem, mlcore::State* s);


/**
 * Performs a Bellman backup of a state, and the￼n updates the state with
 * the resulting expected cost and greedy action.
 *
 * @param problem The problem that contains the given state.
 * @param s The state on which the Bellman backup will be performed.
 * @return The residual of the state.
 */
double bellmanUpdate(mlcore::Problem* problem, mlcore::State* s);


/**
 * Performs a weighted-Bellman backup a state, and then updates the state with
 * the resulting expected cost and greedy action.
 *
 * This backup uses fSSPUDE - see http://arxiv.org/pdf/1210.4875.pdf
 *
 * @param problem The problem that contains the given state.
 * @param s The state on which the Bellman backup will be performed.
 * @param weight The weight to use.
 * @return The residual of the state.
 */
double bellmanUpdate(mlcore::Problem* problem, mlcore::State* s, double weight);


/**
 * Samples a successor state of a state and action using the probabilities
 * defined by the problem's transition function.
 *
 * If the given action is a null pointer or if the state is a dead-end
 * (i.e., the transition function returns empty list of successors) this method
 * will return the same state that is given.
 *
 * @param problem The problem that defines the transition function.
 * @param s The state for which the sucessor state will be sampled.
 * @param a The action that generates the successors.
 * @return A successor sampled from the transition function corresponding to the
 *        state and action pair.
 */
mlcore::State*
randomSuccessor(mlcore::Problem* problem, mlcore::State* s, mlcore::Action* a);


/**
 * Returns the action with minimum Q-value for a state.
 *
 * This method assumes that any action stored in state.bestAction() is
 * consistent with the latest expected costs in the problem. This is
 * guaranteed by any solver that performs backup operations through calls
 * to bellmanUpdate(problem, state).

 * When no action is stored in the state, then action is chosen greedily on the
 * states estimated costs as stored in state.cost().
 *
 * @param problem The problem that contains the given state.
 * @param s The state for which the action will be computed.
 * @return The action with minimum Q-value.
 */
mlcore::Action* greedyAction(mlcore::Problem* problem, mlcore::State* s);


/**
 * Computes the residual of a state.
 *
 * @param problem The problem that contains the given state.
 * @param s The state for which the residual will be computed.
 * @return The residual of the given state.
 */
double residual(mlcore::Problem* problem, mlcore::State* s);


/**
 * Returns the most likely outcome after executing the given action on the given
 * state.
 *
 * @param problem The problem that defines the transition function.
 * @param s The state for which the most likely outcome will be obtained.
 * @param a The action executed in the given state.
 * @return The most likely outcome of the state and action pair.
 */
mlcore::State* mostLikelyOutcome(mlcore::Problem* problem,
                                 mlcore::State* s,
                                 mlcore::Action* a);


/**
 * Samples a trial of the greedy policy implied by the current state
 * values and returns the accumulated discounted cost (using problem->gamma()).
 * The trial starts at the given state s.
 *
 * @param problem The problem that defines the transition function.
 * @param s The initial state for the trial.
 * @return The accumulated cost of this trial.
 */
double sampleTrial(mlcore::Problem* problem, mlcore::State* s);


/**
 * Gets all reachable states starting from initialState in problem,
 * up to the given horizon. The states are stored in the set reachableStates.
 * Returns a boolean variable that is true if a goal is reachable, false
 * otherwise.
 *
 * @param problem The problem describing the state space to traverse.
 * @param initialState The initial state for the search.
 * @param horizon the horizon limit for the search.
 * @param reachableStates A set to store the reachable states in.
 * @param tipStates A set to store the tip states in.
 * @return true if a goal is reachable, false otherwise.
 */
bool getReachableStates(mlcore::Problem* problem,
                        mlcore::State* initialState,
                        int horizon,
                        mlcore::StateSet& reachableStates,
                        mlcore::StateSet& tipStates);

/**
 * Gets all reachable states starting from initialState in problem by following
 * the greedy policy on the state values (i.e., the current best partial
 * solution graph). The states are stored in the set bpsg.
 *
 * @param problem The problem describing the state space to traverse.
 * @param initialState The initial state for the search.
 * @param bpsg A set to store the best partial solution graph.
 */
void getBestPartialSolutionGraph(mlcore::Problem* problem,
                                 mlcore::State* initialState,
                                 mlcore::StateSet& bpsg);

} // mlsolvers


#endif // MDPLIB_SOLVER_H
