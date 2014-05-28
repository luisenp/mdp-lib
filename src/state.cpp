#include "../include/state.h"
#include "../include/problem.h"
#include "../include/mdplib.h"
#include "../include/heuristic.h"
#include <iostream>

std::ostream& operator<<(std::ostream& os, State* s)
{
    s->print(os);
}

Rational State::cost() const
{
    if (cost_ > mdplib::dead_end_cost) {
        if (problem_ == nullptr || problem_->heuristic() == nullptr)
            return Rational(0);
        else
            return problem_->heuristic()->cost(this);
    }
    return cost_;
}
