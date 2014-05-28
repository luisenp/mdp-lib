#include "../../include/solvers/solver.h"
#include "../../include/solvers/LRTDPSolver.h"

LRTDPSolver::LRTDPSolver(Problem* problem)
{
    problem_ = problem;
}

void LRTDPSolver::trial()
{
    State* cur = problem_->initialState();
    while (!problem_->goal(cur)) {
        std::pair<Rational, Action*> best = bellmanBackup(problem_, cur);
        cur = randomSuccessor(problem_, cur, best.bb_action);
    }
}

void LRTDPSolver::solve(int maxTrials)
{
    for (State* s: problem_->states()) { // In case another algorithm modified these values
        s->unvisit();
        s->setBestAction(0);
        s->setCost(0);
    }
    trial();
}
