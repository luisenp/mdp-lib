#include <list>
#include <climits>
#include <cmath>

#include "../../include/solvers/VISolver.h"
#include "../../include/state.h"
#include "../../include/util/general.h"

#include "../../include/domains/gridworld/GridWorldAction.h"

VISolver::VISolver(Problem* problem)
{
    problem_ = problem;
}

void VISolver::solve(int maxIter, Rational tol)
{
    for (int i = 0; i < maxIter; i++) {
        Rational residual(0);
        for (State* s : problem_->states()) {
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
            Rational diff(fabs(s->cost().value() - bestQ.value()));
            if (diff > residual) {
                residual = diff;
            }
            s->setCost(bestQ);
        }
        if (residual < tol)
            return;
    }
}
