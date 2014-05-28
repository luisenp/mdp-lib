#ifndef MDPLIB_SOLVER_H

#include <random>

#include "../../include/problem.h"
#include "../../include/heuristic.h"
#include "../../include/state.h"
#include "../../include/util/rational.h"

inline Rational bellmanBackupHeuristic(Problem* problem, Heuristic* h, State* s)
{
    Rational bestQ(mdplib::dead_end_cost);
    for (Action* a : problem->actions()) {
        if (!problem->applicable(s, a))
            continue;
        std::list<Successor> successors = problem->transition(s, a);
        Rational qAction = problem->cost(s, a);
        for (Successor su : successors) {
            State* s = problem->getState(su.su_state);
            qAction = qAction + su.su_prob * h->cost(problem, s);
        }
        if (qAction < bestQ) {
            bestQ = qAction;
        }
    }
    return bestQ;
}

inline Rational bellmanBackup(Problem* problem, State* s)
{
    Rational bestQ(mdplib::dead_end_cost);
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
        }
    }
    return bestQ;
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
