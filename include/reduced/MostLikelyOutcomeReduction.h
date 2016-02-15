#ifndef MOSTLIKELYOUTCOMEREDUCTION_H
#define MOSTLIKELYOUTCOMEREDUCTION_H

#include <list>
#include <vector>

#include "../State.h"

#include "ReducedTransition.h"

namespace mlreduced
{

class MostLikelyOutcomeReduction : public ReducedTransition
{
private:
    mlcore::Problem* problem_;

public:
    MostLikelyOutcomeReduction(mlcore::Problem* problem) :
        problem_(problem) { }

    virtual ~MostLikelyOutcomeReduction() { }

    virtual
    std::vector<bool> isPrimary(mlcore::State* s, mlcore::Action *a) const
    {
        std::list<mlcore::Successor> successors = problem_->transition(s, a);
        std::vector<bool> primaryValues(successors.size(), false);
        double maximumProbability = 0.0;
        int i = 0;
        int indexMostLikely = 0;
        for (mlcore::Successor successor : successors) {
            if (successor.su_prob > maximumProbability) {
                maximumProbability = successor.su_prob;
                indexMostLikely = i;
            }
            i++;
        }
        primaryValues[indexMostLikely] = true;
        return primaryValues;
    }
};

}
#endif // MOSTLIKELYOUTCOMEREDUCTION_H


