#include "../../../include/domains/binarytree/BinaryTreeState.h"

std::ostream& BinaryTreeState::print(std::ostream& os) const
{
    os << "BTState: " << hashValue();
    return os;
}


bool BinaryTreeState::equals(State* other) const
{
    BinaryTreeState* bts = (BinaryTreeState*) other;
    return *this ==  *bts;
}

int BinaryTreeState::hashValue() const
{
    return (1 << (level_)) + index_ - 1;
}
