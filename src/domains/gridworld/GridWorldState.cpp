#include "../../../include/domains/gridworld/GridWorldState.h"

std::ostream& GridWorldState::print(std::ostream& os) const
{
    os << "(" << x_ << "," << y_ << ")";
    return os;
}

int GridWorldState::hash_value() const
{
    return x_ + 31*y_;
}

int GridWorldState::x() const
{
    return x_;
}

int GridWorldState::y() const
{
    return y_;
}
