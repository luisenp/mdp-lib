#include "../../../include/domains/gridworld/GridWorldState.h"

GridWorldState::GridWorldState(mlcore::Problem* problem, int x, int y) : x_(x), y_(y)
{
    problem_ = problem;
}

std::ostream& GridWorldState::print(std::ostream& os) const
{
    os << "(" << x_ << "," << y_ << ")";
    return os;
}

bool GridWorldState::equals(mlcore::State* other) const
{
    GridWorldState* gws = (GridWorldState*) other;
    return *this ==  *gws;
}

int GridWorldState::hashValue() const
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
