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
            bellmanBackupHeuristic(problem_, heuristic_, cur);
        }
    }
    for (int i = 0; i < 20; i++)
        std::cout << "Succ " << randomSuccessor(problem_, problem_->initialState(), problem_->actions().front()) << std::endl;
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
