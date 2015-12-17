#include "../../../include/solvers/metareasoning/MetareasoningState.h"

MetareasoningState::MetareasoningState(mlcore::State* originalState, int iteration)
{
    baseState_ = originalState;
    iteration_ = iteration;
}

std::ostream& MetareasoningState::print(std::ostream& os) const
{
    os << "[" << baseState_ << "," << iteration_ << "]";
    return os;
}

bool MetareasoningState::equals(mlcore::State* other) const
{
    MetareasoningState* ms = (MetareasoningState*) other;
    return *this ==  *ms;
}


int MetareasoningState::hashValue() const
{
    return 31 * baseState_->hashValue() + iteration_;
}
