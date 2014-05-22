#include "../../../include/domains/gridworld/GridWorldAction.h"
#include "../../../include/domains/gridworld/GridWorldProblem.h"

std::ostream& GridWorldAction::print(std::ostream& os) const
{
    os << "action ";
    if (dir_ == gridworld::UP)
        os << "up";
    if (dir_ == gridworld::DOWN)
        os << "down";
    if (dir_ == gridworld::LEFT)
        os << "left";
    if (dir_ == gridworld::RIGHT)
        os << "right";
    return os;
}
