#ifndef MDPLIB_SOLVER_H

#include <random>

#include "../../include/problem.h"
#include "../../include/heuristic.h"
#include "../../include/state.h"
#include "../../include/util/rational.h"

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
    Rational bestQ(mdplib::dead_end_cost);
    Action *bestAction = 0;
    for (Action* a : problem->actions()) {
        if (!problem->applicable(s, a))
            continue;
        std::list<Successor> successors = problem->transition(s, a);
        Rational qAction = problem->cost(s, a);
        for (Successor su : successors) {
            State* s = problem->getState(su.su_state);
            qAction = qAction + su.su_prob * s->cost();
        }
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
