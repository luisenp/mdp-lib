#include "../include/state.h"
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
        if (heuristic_ == 0)
            return Rational(0);
        else
            return heuristic_->cost(this);
    }
    return cost_;
}
