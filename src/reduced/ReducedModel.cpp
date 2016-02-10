#include <vector>

#include "../../include/reduced/ReducedModel.h"

namespace mlreduced
{

std::list<mlcore::Successor>
ReducedModel::transition(mlcore::State* s, mlcore::Action *a)
{
    ReducedState* rs = (ReducedState *) s;
    std::vector<bool>& isPrimary =
        config_->isPrimary(rs->originalState(), a);

    std::list<mlcore::Successor> successors;
    std::list<mlcore::Successor> originalSuccessors =
        originalProblem_->transition(rs->originalState(), a);
    double totalPrimaryProbability = 0.0;
    int i = 0;
    for (mlcore::Successor origSucc : originalSuccessors) {
        mlcore::State* next = nullptr;
        if (isPrimary[i]) {
            totalPrimaryProbability += origSucc.su_prob;
            next = new ReducedState(origSucc.su_state,
                                    rs->exceptionCount());
        } else {
            next = new ReducedState(origSucc.su_state,
                                    rs->exceptionCount() + 1);
        }
        if (rs->exceptionCount() < k_ || isPrimary[i]) {
            successors.push_back(mlcore::Successor(next, origSucc.su_prob));
        }
        i++;
    }

    if (rs->exceptionCount() == k_) {
        for (mlcore::Successor successor : successors) {
            successor.su_prob /= totalPrimaryProbability;
        }
    }
    return successors;
}

}

