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
    LeastLikelyOutcomeReduction(mlcore::Problem* problem) :
        problem_(problem) { }

    virtual ~LeastLikelyOutcomeReduction() { }

    /**
     * Overrides method from ReducedTransition.
     */
    virtual void setPrimary(mlcore::State* s,
                            mlcore::Action *a,
                            std::vector<bool>& primaryIndicators) const
    {
        std::list<mlcore::Successor> successors = problem_->transition(s, a);
        double minimumProbability = 2.0;
        int i = 0;
        int indexLeastLikely = 0;
        for (mlcore::Successor successor : successors) {
            primaryIndicators.push_back(false);
            if (successor.su_prob <= minimumProbability) {
                minimumProbability = successor.su_prob;
                indexLeastLikely = i;
            }
            i++;
        }
        primaryIndicators[indexLeastLikely] = true;
    }
};

}
#endif // LEASTLIKELYOUTCOMEREDUCTION_H

