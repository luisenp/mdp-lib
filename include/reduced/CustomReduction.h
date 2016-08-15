#ifndef MDPLIB_CUSTOMREDUCTION_H
#define MDPLIB_CUSTOMREDUCTION_H

#include <unordered_map>

#include "ReducedTransition.h"


namespace mlreduced
{

class CustomReduction : public ReducedTransition
{
private:
    std::unordered_map<a, std::vector<bool> > primaryIndicatorsActions;

public:
    CustomReduction(mlcore::Problem* problem) {
        for (mlcore::Action* action : problem->actions()) {
            primaryIndicatorsActions[action] = vector<bool> ();
        }
    }

    virtual ~CustomReduction() {}

    /**
     * Sets the primary outcomes indicators for the given action according to
     * the given vector of booleans.
     */
    virtual void setPrimaryForAction(mlcore::Action* a,
                                     std::vector<bool> primaryIndicators)
    {
        assert(primaryIndicatorsActions.count(a) > 0);
        primaryIndicatorsActions[a] = primaryIndicators;
    }

    /**
     * Overrides method from ReducedTransition.
     */
    virtual void setPrimary(mlcore::State* s,
                            mlcore::Action *a,
                            std::vector<bool>& primaryIndicators) const
    {
        primaryIndicators = primaryIndicatorsActions[a];
    }
};

}
#endif // MDPLIB_CUSTOMREDUCTION_H
