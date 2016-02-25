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

    /**
     * Overrides method from ReducedTransition.
     */
    virtual void setPrimary(mlcore::State* s,
                            mlcore::Action *a,
                            std::vector<bool>& primaryIndicators) const
    {
        std::list<mlcore::Successor> successors = problem_->transition(s, a);
        double maximumProbability = 0.0;
        int i = 0;
        int indexMostLikely = 0;
        for (mlcore::Successor successor : successors) {
            primaryIndicators.push_back(false);
            if (successor.su_prob > maximumProbability) {
                maximumProbability = successor.su_prob;
                indexMostLikely = i;
            }
            i++;
        }
        primaryIndicators[indexMostLikely] = true;
    }
};

}
#endif // MOSTLIKELYOUTCOMEREDUCTION_H


