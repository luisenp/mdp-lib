#include "../../include/solvers/solver.h"
#include "../../include/solvers/LRTDPSolver.h"

LRTDPSolver::LRTDPSolver(Problem* problem)
{
    problem_ = problem;
}

void LRTDPSolver::trial(Rational epsilon)
{
    State* tmp = problem_->initialState();
    std::list<State*> visited;
    while (!tmp->checkBits(mdplib::SOLVED)) {
        visited.push_front(tmp);
        if (problem_->goal(tmp))
            break;
        bellmanUpdate(problem_, tmp);
        tmp = randomSuccessor(problem_, tmp, tmp->bestAction());
    }

    while (!visited.empty()) {
        tmp = visited.front();
        visited.pop_front();
        if (!checkSolved(tmp, epsilon))
            break;
    }
}

bool LRTDPSolver::checkSolved(State* s, Rational epsilon)
{
    bool rv = true;
    std::list<State*> open, closed;
    StateSet openSet, closedSet;

    State* tmp = s;
    if (!tmp->checkBits(mdplib::SOLVED)) {
        open.push_front(s);
        openSet.insert(s);
    }

    while (!open.empty()) {
        tmp = open.front();
        open.pop_front();
        openSet.erase(tmp);
        closed.push_front(tmp);
        closedSet.insert(tmp);

        if (residual(problem_, tmp) > epsilon) {
            rv = false;
            continue;
        }

        Action* a = greedyAction(problem_, tmp);
        std::list<Successor> successors = problem_->transition(tmp, a);
        for (Successor su : successors) {
            State* next = su.su_state;
            if (!next->checkBits(mdplib::SOLVED)
                && openSet.find(next) == openSet.end()
                && closedSet.find(next) == closedSet.end())
                open.push_front(next);
                openSet.insert(next);
        }
    }

    if (rv) {
        for (State* sc : closed)
            sc->setBits(mdplib::SOLVED);
    } else {
        while (!closed.empty()) {
            tmp = closed.front();
            closed.pop_front();
            bellmanUpdate(problem_, tmp);
        }
    }

    return rv;
}

void LRTDPSolver::solve(int maxTrials, Rational epsilon)
{
    for (State* s: problem_->states())  // In case another algorithm modified these values
        s->reset();

    int trials = 0;
    while (!problem_->initialState()->checkBits(mdplib::SOLVED) && trials++ < maxTrials)
        trial(epsilon);
}
