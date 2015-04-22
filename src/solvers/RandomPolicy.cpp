#include "../solvers/RandomPolicy.h"

#include <cassert>

namespace mlsolvers
{

void RandomPolicy::addActionsState(mlcore::State* s, std::vector<double> actions)
{
    if (stateIndex_.count(s) == 0)
        stateIndex_[s] = stateCounter_++;


    int idxState = stateIndex_[s];

    assert(idxState < policy_.size());

    policy_[idxState] = actions;
}

mlcore::Action* RandomPolicy::getRandomAction(mlcore::State* s)
{
    assert(stateIndex_.count(s) != 0);

    int idxState = stateIndex_[s];
    std::vector<double>& actions = policy_[idxState];

    double pick = dis(gen);
    double acc = 0.0;
    int i = 0;
    for (mlcore::Action* a : problem_->actions()) {
        acc += actions[i];
        if (acc >= pick)
            return a;
        i++;
    }

    dprint3(s, " ", acc);
    return nullptr;
}

} // namespace mlsolvers
