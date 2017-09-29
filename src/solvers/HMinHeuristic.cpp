#include<list>

#include "../include/MDPLib.h"
#include "../include/State.h"

#include "../../include/solvers/HMinHeuristic.h"
#include "../../include/solvers/Solver.h"


using namespace mlcore;
using namespace std;

namespace mlsolvers
{

double HMinHeuristic::hminQvalue(mlcore::State* s, mlcore::Action* a)
{
    double qAction = problem_->cost(s, a);
    double costSuccessor = mdplib::dead_end_cost;
    for (auto const & successor : problem_->transition(s, a)) {
        costSuccessor = std::min(costSuccessor, costs_[successor.su_state]);
    }
    qAction += costSuccessor;
    return std::min(qAction, mdplib::dead_end_cost);
}


void
HMinHeuristic::hminUpdate(State* s)
{
    if (problem_->goal(s)) {
        costs_[s] = 0.0;
        return;
    }

    double bestQ = mdplib::dead_end_cost;
    bool hasAction = false;
    for (Action* a : problem_->actions()) {
        if (!problem_->applicable(s, a))
            continue;
        hasAction = true;
        double qAction = problem_->cost(s, a);
        double minCostSuccessor = mdplib::dead_end_cost;
        for (auto const & successor : problem_->transition(s, a)) {
            minCostSuccessor =
                std::min(minCostSuccessor, costs_[successor.su_state]);
        }
        qAction += minCostSuccessor;
        qAction = std::min(qAction, mdplib::dead_end_cost);
        if (qAction <= bestQ) {
            bestQ = qAction;
            bestActions_[s] = a;
        }
    }
    if (!hasAction) {
        s->markDeadEnd();
    }
    costs_[s] = bestQ;
}


HMinHeuristic::HMinHeuristic(Problem* problem, bool solveAll)
{
    problem_ = problem;
    solveAll_ = solveAll;
    if (solveAll_) {
        problem->generateAll();
        while (true) {
            double maxResidual = 0.0;
            for (State * s : problem->states()) {
                double prevCost = costs_[s];
                hminUpdate(s);
                maxResidual = std::max(maxResidual, fabs(costs_[s]- prevCost));
            }
            if (maxResidual < 1.0e-6)
                break;
        }
    }
}


double HMinHeuristic::cost(const State* s)
{
    if (problem_->goal(const_cast<State*>(s)))
        return 0.0;

    if (s->checkBits(mdplib::SOLVED_HMIN))
        return costs_.at(const_cast<State*>(s));

    State* currentState = nullptr;
    while (true) {
        // Starting a LRTA* trial.
        currentState = const_cast<State*> (s);
        list<State*> visited;
        visited.push_back(currentState);
        while (!problem_->goal(currentState)) {
            hminUpdate(currentState);
            if (currentState->deadEnd())
                break;
            // Getting the successor of the best action.
                                                                                if (hminSuccessor(currentState, bestActions_.at(currentState)) == nullptr) {
                                                                                    dprint("successsor is null", currentState);
                                                                                }
            currentState = hminSuccessor(currentState,
                                         bestActions_.at(currentState));
            visited.push_back(currentState);
        }

        // Checking if the state was solved.
        while (!visited.empty()) {
            currentState = visited.back(); visited.pop_back();
            if (!checkSolved(currentState))
                break;
        }

        if (currentState->checkBits(mdplib::SOLVED_HMIN))
            break;
    }

    return costs_.at(const_cast<State*>(s));
}


State* HMinHeuristic::hminSuccessor(State* s, Action* a)
{
    double minCost = mdplib::dead_end_cost;
    State* minCostSuccessor = nullptr;
    for (auto const & successor : problem_->transition(s, a)) {
        double successorCost = costs_.at(successor.su_state);
        if (successorCost <= minCost) {
            minCostSuccessor = successor.su_state;
            minCost = successorCost;
        }
    }
    return minCostSuccessor;
}


bool HMinHeuristic::checkSolved(State* s)
{
    std::list<State*> open, closed;

    State* tmp = s;
    if (!tmp->checkBits(mdplib::SOLVED_HMIN)) {
        open.push_front(s);
    }

    bool rv = true;
    while (!open.empty()) {
        tmp = open.front();
        open.pop_front();

        if (problem_->goal(tmp))
            continue;

        if (tmp->deadEnd())
            continue;

        closed.push_front(tmp);
        tmp->setBits(mdplib::CLOSED);

        if (bestActions_.count(tmp) == 0) {
            rv = false;
            break;
        }
        Action* a = bestActions_.at(tmp);

        if (fabs(costs_[tmp] - hminQvalue(tmp, a)) > 1.0e-3)
            rv = false;

        State* next = hminSuccessor(tmp, a);
        if (!next->checkBits(mdplib::SOLVED) &&
                !next->checkBits(mdplib::CLOSED)) {
            open.push_front(next);
        }
    }

    if (rv) {
        for (State* sc : closed) {
            sc->setBits(mdplib::SOLVED_HMIN);
            sc->clearBits(mdplib::CLOSED);
        }
    } else {
        while (!closed.empty()) {
            tmp = closed.front();
            closed.pop_front();
            tmp->clearBits(mdplib::CLOSED);
            hminUpdate(tmp);
        }
    }

    return rv;
}

} // namespace mlsovers
