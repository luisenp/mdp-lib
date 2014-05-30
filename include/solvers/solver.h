#ifndef MDPLIB_SOLVER_H

#include <random>
#include <cassert>

#include "../../include/problem.h"
#include "../../include/heuristic.h"
#include "../../include/state.h"
#include "../../include/util/rational.h"

#define bb_cost first
#define bb_action second

/**
 * Computes the Q-value of a state-action pair.
 * This method assumes that the given action is applicable on the state.
 *
 * @param problem The problem that contains the given state.
 * @param s The state for which the Q-value will be computed.
 * @param a The action for which the Q-value will be computed
 * @return The Q-value of the state-action pair.
 */
inline Rational qvalue(Problem* problem, State*s, Action* a)
{
    std::list<Successor> successors = problem->transition(s, a);
    Rational qAction = problem->cost(s, a);
    for (Successor su : successors) {
        State* s = problem->addState(su.su_state);
        qAction = qAction + su.su_prob * s->cost();
    }
    return qAction;
}

/**
 * Performs a Bellman backup a state.
 *
 * This backup uses fSSPUDE - see http://arxiv.org/pdf/1210.4875.pdf
 *
 * @param problem The problem that contains the given state.
 * @param s The state on which the Bellman backup will be performed.
 * @return A pair containing the estimated cost and estimated best action according
 *        to this Bellman backup.
 */
inline std::pair<Rational, Action*> bellmanBackup(Problem* problem, State* s)
{
    Rational bestQ(mdplib::dead_end_cost);
    Action *bestAction = 0;
    for (Action* a : problem->actions()) {
        if (!problem->applicable(s, a))
            continue;
        Rational qAction = qvalue(problem, s, a);
        if (qAction < bestQ) {
            bestQ = qAction;
            bestAction = a;
        }
    }
    return std::make_pair(bestQ, bestAction);
}

/**
 * Performs a Bellman backup a state, and then updates the state with
 * the resulting expected cost and greedy action.
 *
 * @param problem The problem that contains the given state.
 * @param s The state on which the Bellman backup will be performed.
 * @return The residual of the state.
 */
inline Rational bellmanUpdate(Problem* problem, State* s)
{
    std::pair<Rational, Action*> best = bellmanBackup(problem, s);
    Rational residual = s->cost() - best.bb_cost;
    s->setCost(best.bb_cost);
    s->setBestAction(best.bb_action);
    return residual.abs();
}


/**
 * Samples a successor state of a state and action using the probabilities
 * defined by the problem's transition function.
 *
 * @param problem The problem that contains the given state.
 * @param s The state for which the sucessor state will be sampled.
 * @param a The action that generates the successors.
 * @return A successor sampled from the transition function corresponding to the
 *        state and action pair.
 */
inline State* randomSuccessor(Problem* problem, State* s, Action* a)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);
    double pick = dis(gen);

    Rational acc(0);
    std::list<Successor> successors = problem->transition(s, a);
    assert(!successors.empty());
    for (Successor sccr : successors) {
        acc = acc + sccr.su_prob;
        if (acc.value() >= pick)
            return sccr.su_state;
    }
}

/**
 * Returns the action with minimum Q-value for a state.
 *
 * This method assumes that any action stored state.bestAction() is consistent with
 * the latest expected costs in the problem. This is guaranteed by any solver that
 * performs backup operations through calls to bellmanUpdate(problem, state).

 * When no action is stored in the state, then a Bellman backup is performed to
 * compute the action with minimum Q-value.
 *
 *
 * @param problem The problem that contains the given state.
 * @param s The state for which the action will be computed.
 * @return The action with minimum Q-value.
 */
inline Action* greedyAction(Problem* problem, State* s)
{
    Action* a = s->bestAction();
    if (a == nullptr)
        return bellmanBackup(problem, s).bb_action;
    return a;
}

/**
 * Computes the residual of a state.
 *
 * @param problem The problem that contains the given state.
 * @param s The state for which the residual will be computed.
 * @return The residual of the given state.
 */
inline Rational residual(Problem* problem, State* s)
{
    Action* bestAction = greedyAction(problem, s);
    Rational res = qvalue(problem, s, bestAction) - s->cost();
    return res.abs();
}

#endif // MDPLIB_SOLVER_H
