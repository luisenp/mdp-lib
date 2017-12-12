#include "../solvers/RandomPolicy.h"

#include <cassert>

namespace mlsolvers
{

void RandomPolicy::addActionsState(mlcore::State* s,
                                   std::vector<double> actions)
{
    if (stateIndex_.count(s) == 0)
        stateIndex_[s] = stateCounter_++;


    int idxState = stateIndex_[s];

    assert(idxState < policy_.size());

    policy_[idxState] = actions;

    double total = 0.0;
    for (int i = 0; i < actions.size(); i++) {
        double d = actions[i];
        total += d;
    }
}


mlcore::Action* RandomPolicy::getRandomAction(mlcore::State* s)
{
    assert(stateIndex_.count(s) != 0);

    int idxState = stateIndex_[s];
    std::vector<double>& actions = policy_[idxState];

    double pick = kUnif_0_1(kRNG);
    double acc = 0.0;
    int i = 0;
    for (mlcore::Action* a : problem_->actions()) {
        acc += actions[i];
        if (acc >= pick)
            return a;
        i++;
    }
    return nullptr;
}


void RandomPolicy::print(std::ostream& os)
{
    for (mlcore::State* s : problem_->states()) {
        os << s << std::endl;
        int idxState = stateIndex_[s];
        int idxAction = 0;
        for (mlcore::Action* a : problem_->actions()) {
            if (policy_[idxState][idxAction] > 0.0) {
                os << "    " << a << " " <<
                    policy_[idxState][idxAction] << " " << std::endl;
            }
            idxAction++;
        }
    }
}


double RandomPolicy::getValue(mlcore::State* s)
{
    return values_[s];
}

} // namespace mlsolvers
