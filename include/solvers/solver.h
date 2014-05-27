#ifndef MDPLIB_SOLVER_H

#include "../../include/problem.h"
#include "../../include/state.h"
#include "../../include/util/rational.h"

Rational bellmanBackup(Problem* problem_, State* s)
{
    Rational bestQ(mdplib::dead_end_cost);
    for (Action* a : problem_->actions()) {
        if (!problem_->applicable(s, a))
            continue;
        std::list<Successor> successors = problem_->transition(s, a);
        Rational qAction = problem_->cost(s, a);
        for (Successor su : successors) {
            State* s = problem_->getState(su.first);
            qAction = qAction + su.second * s->cost();
        }
        if (qAction < bestQ) {
            bestQ = qAction;
        }
    }
    return bestQ;
}

#endif // MDPLIB_SOLVER_H
