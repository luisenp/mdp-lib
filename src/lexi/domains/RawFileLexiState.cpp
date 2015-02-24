#include "../../../include/lexi/domains/RawFileLexiState.h"

namespace mllexi
{

void RawFileLexiState::addSuccessor(int actionId, RawFileLexiState* succState, double prob)
{
    allSuccessors_[actionId].push_back(mlcore::Successor(succState, prob));
}

mlcore::SuccessorsList& RawFileLexiState::getSuccessors(int actionId)
{
    return allSuccessors_[actionId];
}

void RawFileLexiState::setCostAction(int actionId, int level, double cost)
{
    costs_[level][actionId] = cost;
}

double RawFileLexiState::getCostAction(int actionId, int level)
{
    return costs_[level][actionId];
}

std::ostream& RawFileLexiState::print(std::ostream& os) const
{
    os << "Raw File State: " << id_;
    return os;
}

mlcore::State& RawFileLexiState::operator=(const mlcore::State& rhs)
{
    if (this == &rhs)
        return *this;

    RawFileLexiState* state = (RawFileLexiState*)  & rhs;
    id_ = state->id_;
    allSuccessors_ = state->allSuccessors_;
    return *this;
}

bool RawFileLexiState::operator==(const mlcore::State& rhs) const
{
    RawFileLexiState* state = (RawFileLexiState*)  & rhs;
    return id_ == state->id_;
}

bool RawFileLexiState::equals(mlcore::State* other) const
{
    RawFileLexiState* state = (RawFileLexiState*) other;
    return *this ==  *state;
}

int RawFileLexiState::hashValue() const
{
    return id_;
}

}
