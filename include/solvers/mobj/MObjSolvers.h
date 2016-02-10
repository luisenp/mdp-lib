#ifndef MDPLIB_MOBJ_SOLVER_H
#define MDPLIB_MOBJ_SOLVER_H

#include <vector>

#include "../Heuristic.h"
#include "../Problem.h"
#include "../State.h"
#include "../util/general.h"

#include "../../mobj/MObjProblem.h"
#include "../../mobj/MObjState.h"

namespace mdplib_mobj_solvers
{

/**
 * Computes the Q-value of a state-action pair for the given
 * multi-objective MDP. The method receives the index of the value function
 * according to which the Q-value will be computed.
 *
 * This method assumes that the given action is applicable on the state.
 *
 * @param problem The problem that contains the given state.
 * @param s The state for which the Q-value will be computed.
 * @param a The action for which the Q-value will be computed.
 * @param i The index of the value function to use.
 * @return The Q-value of the state-action pair.
 */
double
qvalue(mlmobj::MOProblem* problem, mlmobj::MOState* s, mlcore::Action* a, int i);


/**
 * Performs a Lexicographic Bellman update of a state according to the
 * level-th cost function. The operator assumes that all values from 1 to
 * (level - 1) are correct.
 *
 * This backup uses fSSPUDE - see http://arxiv.org/pdf/1210.4875.pdf
 *
 * @param problem The problem that contains the given state.
 * @param s The state on which the Bellman backup will be performed.
 * @param level The level of the cost function to be miminized.
 * @return The maximum residual among all value functions.
 */
double
lexiBellmanUpdate(mlmobj::MOProblem* problem, mlmobj::MOState* s, int level);


/**
 * Bellman update for MOSSPs using linear combinations of the cost functions.
 *
 * Performs a Bellman backup of a state, and then updates the state with
 * the resulting expected cost and greedy action.
 *
 * @param problem The problem that contains the given state.
 * @param s The state on which the Bellman backup will be performed.
 * @return The residual of the state.
 */
double bellmanUpdate(mlmobj::MOProblem* problem, mlmobj::MOState* s);


/**
 * Samples a trial of the greedy policy implied by state->bestAction()
 * and returns the accumulated discounted cost (using problem->gamma()) over
 * all cost functions. The trial starts at the given state s.
 *
 * @param problem The multi-objective problem that defines the transitions.
 * @param s The initial state for the trial.
 * @return The accumulated costs of this trial.
 */
std::vector<double> sampleTrial(mlmobj::MOProblem* problem, mlcore::State* s);

} // mdplib_mobj_solvers


#endif // MDPLIB_MOBJ_SOLVER_H
