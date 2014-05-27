#include "../../include/solvers/solver.h"
#include "../../include/solvers/LRTDPSolver.h"

LRTDPSolver::LRTDPSolver(Problem* problem, Heuristic* heuristic)
{
    problem_ = problem;
    heuristic_ = heuristic;
}

void LRTDPSolver::trial()
{
    for (int i = 0; i < 20; i++)
        std::cout << "Succ " << randomSuccessor(problem_, problem_->initialState(), problem_->actions().front()) << std::endl;
}

void LRTDPSolver::solve(int maxTrials)
{
    trial();
}
