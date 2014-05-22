#include "../../include/solvers/VISolver.h"

VISolver::VISolver(Problem* problem)
{
    problem_ = problem;
}

void VISolver::solve()
{
    for (State* s : problem_->states()) {
        for (Action* a : problem_->actions()) {
            if (!problem_->applicable(s, a))
                continue;

        }
    }
}
