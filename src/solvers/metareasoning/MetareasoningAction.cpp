#include "../../../include/solvers/metareasoning/MetareasoningAction.h"

std::ostream& MetareasoningAction::print(std::ostream& os) const
{
    os << "Meta-Action: " << (isNOP_ ? "think" : "act");
}
