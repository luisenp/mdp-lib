#include "../../include/solvers/solver.h"
#include "../../include/solvers/LRTDPSolver.h"

LRTDPSolver::LRTDPSolver(Problem* problem)
{
    problem_ = problem;
}

void LRTDPSolver::trial()
{
    State* tmp = problem_->initialState();
    while (!problem_->goal(tmp)) {
        std::pair<Rational, Action*> best = bellmanBackup(problem_, tmp);
        tmp->setCost(best.bb_cost);
        tmp->setBestAction(best.bb_action);
        tmp = randomSuccessor(problem_, tmp, best.bb_action);
    }
}

void LRTDPSolver::checkSolved(State* s)
{
    bool rv = true;
    std::list<State*> open;
    StateSet visited;

    State* tmp = s;
    if (!tmp->checkBits(mdplib::SOLVED))
        open.push_front(s);

    while (!open.empty()) {
        tmp = open.front();
        open.pop_front();

        if (!visited.insert(tmp).second) // state was already visited
            continue;
    }
}

void LRTDPSolver::solve(int maxTrials)
{
    for (State* s: problem_->states())  // In case another algorithm modified these values
        s->reset();

    for (int i = 0; i < maxTrials; i++)
        trial();
}
