#include "../include/State.h"
#include "../include/Problem.h"
#include "../include/solvers/Solver.h"
#include "../include/MDPLib.h"
#include "../include/Heuristic.h"
#include <iostream>

namespace mlcore
{

std::ostream& operator<<(std::ostream& os, State* s)
{
    return s->print(os);
}

double State::cost() const
{
    if (deadEnd())
        return mdplib::dead_end_cost;

    if (cost_ > mdplib::dead_end_cost) {
        if (problem_ == nullptr || problem_->heuristic() == nullptr)
            return 0.0;
        else
            return problem_->heuristic()->cost(this);
    }
    return cost_;
}

double State::gValue() const
{
    if (gValue_ > mdplib::dead_end_cost)
        return 0.0;
    return gValue_;
}

double State::hValue() const
{
    if (hValue_ > mdplib::dead_end_cost) {
        if (problem_ == nullptr || problem_->heuristic() == nullptr)
            return 0.0;
        else
            return problem_->heuristic()->cost(this);
    }
    return hValue_;
}

}

