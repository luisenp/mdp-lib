#include "../../../include/mobj/domains/RawFileMOState.h"

namespace mlmobj
{

void RawFileMOState::addSuccessor(int actionId, RawFileMOState* succState, double prob)
{
    allSuccessors_[actionId].push_back(mlcore::Successor(succState, prob));
}

mlcore::SuccessorsList& RawFileMOState::getSuccessors(int actionId)
{
    return allSuccessors_[actionId];
}

void RawFileMOState::setCostAction(int actionId, int level, double cost)
{
    costs_[level][actionId] = cost;
}

double RawFileMOState::getCostAction(int actionId, int level)
{
    return costs_[level][actionId];
}

std::ostream& RawFileMOState::print(std::ostream& os) const
{
    os << "Raw File State: " << id_;
    return os;
}

mlcore::State& RawFileMOState::operator=(const mlcore::State& rhs)
{
    if (this == &rhs)
        return *this;

    RawFileMOState* state = (RawFileMOState*)  & rhs;
    id_ = state->id_;
    allSuccessors_ = state->allSuccessors_;
    return *this;
}

bool RawFileMOState::operator==(const mlcore::State& rhs) const
{
    RawFileMOState* state = (RawFileMOState*)  & rhs;
    return id_ == state->id_;
}

bool RawFileMOState::equals(mlcore::State* other) const
{
    RawFileMOState* state = (RawFileMOState*) other;
    return *this ==  *state;
}

int RawFileMOState::hashValue() const
{
    return id_;
}

}
