#include "../../include/solvers/solver.h"
#include "../../include/solvers/LRTDPSolver.h"

LRTDPSolver::LRTDPSolver(Problem* problem, Heuristic* heuristic)
{
    problem_ = problem;
    heuristic_ = heuristic;
}

void LRTDPSolver::trial()
{
    State* cur = problem_->initialState();
    while (!problem_->goal(cur)) {
        if (cur->bestAction() == 0) { // hasn't been expanded so far
            bellmanBackup(problem_, cur);
        }
        break;
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
