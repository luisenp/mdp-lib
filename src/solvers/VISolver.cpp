#include <list>
#include <climits>

#include "../../include/solvers/VISolver.h"
#include "../../include/state.h"
#include "../../include/util/general.h"

#include "../../include/domains/gridworld/GridWorldAction.h"

VISolver::VISolver(Problem* problem)
{
    problem_ = problem;
}

void VISolver::solve(int maxIter)
{
    for (int i = 0; i < maxIter; i++) {
        for (State* s : problem_->states()) {
            Rational bestQ(mdplib::dead_end_cost);
            std::cout << s << std::endl;
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
                std::cout << qAction << " " << bestQ << std::endl;
            }
            s->setCost(bestQ);
        }

        for (State* s : problem_->states())
            std::cout << s << " " << s->cost() << std::endl;
        std::cout << "**************************" << std::endl;
    }
}
