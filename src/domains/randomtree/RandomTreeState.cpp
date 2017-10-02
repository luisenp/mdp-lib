#include "../../../include/domains/randomtree/RandomTreeState.h"

#include <cassert>

RandomTreeState::RandomTreeState(int index, int depth, bool is_goal) :
    index_(index), depth_(depth), is_goal_(is_goal) {
}

void RandomTreeState::addAction(mlcore::Action* action) {
    assert(action_successors_.count(action) == 0);
    action_successors_[action] = mlcore::SuccessorsList();
}

void RandomTreeState::addSuccessorForAction(RandomTreeState* state,
                                            double prob,
                                            mlcore::Action* action) {
    assert(action_successors_.count(action) != 0);
    action_successors_[action].push_back(mlcore::Successor(state, prob));
}

mlcore::State& RandomTreeState::operator=(const mlcore::State& rhs) {
    if (this == &rhs)
        return *this;
    const RandomTreeState* rts = static_cast<const  RandomTreeState*>(&rhs);
    index_ =  rts->index_;
    is_goal_ = rts->is_goal_;
    depth_ = rts->depth_;
    for (auto rts_action_successors : rts->action_successors_) {
        action_successors_[rts_action_successors.first] =
            mlcore::SuccessorsList();
        for (mlcore::Successor sccr : rts_action_successors.second) {
            action_successors_[rts_action_successors.first].push_back(sccr);
        }
    }
    return *this;
}

bool RandomTreeState::operator==(const mlcore::State& rhs) const {
    const RandomTreeState* rts = static_cast<const RandomTreeState*>(&rhs);
    return index_ == rts->index_;
}

bool RandomTreeState::equals(mlcore::State* other) const {
    RandomTreeState* state = static_cast<RandomTreeState *>(other);;
    return *this == *state;
}

int RandomTreeState::hashValue() const {
    return index_;
}
