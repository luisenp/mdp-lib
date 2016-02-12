#ifndef LEASTLIKELYOUTCOMEREDUCTION_H
#define LEASTLIKELYOUTCOMEREDUCTION_H

#include <list>
#include <vector>

#include "../State.h"

#include "ReducedTransition.h"

namespace mlreduced
{

class LeastLikelyOutcomeReduction : public ReducedTransition
{
private:
    mlcore::Problem* problem_;

public:
    LeastLikelyOutcomeReduction(mlcore::Problem* problem) : problem_(problem) { }

    virtual ~LeastLikelyOutcomeReduction() { }

    virtual
    std::vector<bool> isPrimary(mlcore::State* s, mlcore::Action *a) const
    {
        std::list<mlcore::Successor> successors = problem_->transition(s, a);
        std::vector<bool> primaryValues(successors.size(), false);
        double minimumProbability = 2.0;
        int i = 0;
        int indexLeastLikely = 0;
        for (mlcore::Successor successor : successors) {
            if (successor.su_prob < minimumProbability) {
                minimumProbability = successor.su_prob;
                indexLeastLikely = i;
            }
            i++;
        }
        primaryValues[indexLeastLikely] = true;
        return primaryValues;
    }
};

}
#endif // LEASTLIKELYOUTCOMEREDUCTION_H

