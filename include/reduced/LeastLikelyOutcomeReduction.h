#ifndef LEASTLIKELYOUTCOMEREDUCTION_H
#define LEASTLIKELYOUTCOMEREDUCTION_H

#include <algorithm>
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

    /* How many of the least likely outcomes to choose. */
    int howMany_;

    /*
     * Used to compare two successors according to their probability, while
     * keeping information about their position in the successor list.
     */
    static bool succAndIndexCompare(
        const std::pair<mlcore::Successor, int> & succAndIndex1,
        const std::pair<mlcore::Successor, int> & succAndIndex2) {
            return succAndIndex1.first.su_prob < succAndIndex2.first.su_prob;
    }


public:
    LeastLikelyOutcomeReduction(mlcore::Problem* problem, int howMany = 1) :
        problem_(problem), howMany_(howMany) { }

    virtual ~LeastLikelyOutcomeReduction() { }

    /**
     * Overrides method from ReducedTransition.
     */
    virtual void setPrimary(mlcore::State* s,
                            mlcore::Action *a,
                            std::vector<bool>& primaryIndicators) const
    {

        std::vector<std::pair<mlcore::Successor, int> > successorsAndIndices;
        int idx = 0;
        for (auto const & successor : problem_->transition(s, a)) {
            successorsAndIndices.push_back(std::make_pair(successor, idx++));
            primaryIndicators.push_back(false);
        }
        std::sort(successorsAndIndices.begin(),
                  successorsAndIndices.end(),
                  succAndIndexCompare);

        for (int i = 0; i < howMany_ && i < successorsAndIndices.size(); i++) {
            primaryIndicators[successorsAndIndices[i].second] = true;
        }
    }
};

}
#endif // LEASTLIKELYOUTCOMEREDUCTION_H

