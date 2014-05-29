#ifndef MDPLIB_SOLVER_H

#include <random>

#include "../../include/problem.h"
#include "../../include/heuristic.h"
#include "../../include/state.h"
#include "../../include/util/rational.h"

#define bb_cost first
#define bb_action second

/**
 * Computes the Q-value of a given state-action pair on a given problem.
 * This method assumes that the action is applicable on the state.
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
        State* s = problem->getState(su.su_state);
        qAction = qAction + su.su_prob * s->cost();
    }
    return qAction;
}

/**
 * Performs a Bellman backup of the state given as the second paramenter on the
 * problem given as first parameter.
 *
 * @param problem The problem that contains the given state.
 * @param s The state on which the Bellman backup will be performed.
 * @return A pair containing the estimated cost and estimated best action according
 *        to this Bellman backup.
 */
inline std::pair<Rational, Action*> bellmanBackup(Problem* problem, State* s)
{
    /* Note that this backup uses fSSPUDE - see http://arxiv.org/pdf/1210.4875.pdf */
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

inline State* randomSuccessor(Problem* problem, State* s, Action* a)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);
    double pick = dis(gen);

    Rational acc(0);
    std::list<Successor> successors = problem->transition(s, a);
    for (Successor sccr : successors) {
        acc = acc + sccr.su_prob;
        if (acc.value() >= pick)
            return sccr.su_state;
    }
}

#endif // MDPLIB_SOLVER_H
