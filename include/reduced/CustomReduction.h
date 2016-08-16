#ifndef MDPLIB_CUSTOMREDUCTION_H
#define MDPLIB_CUSTOMREDUCTION_H

#include <unordered_map>
#include <vector>

#include "../Action.h"
#include "../State.h"

#include "ReducedTransition.h"


namespace mlreduced
{

/**
 * This class allows to create custom reduced models by manually setting which
 * outcomes will be considered primary for each action or state. If a set
 * of primary outcomes have been defined for a state, this takes precedence
 * over any set of primary outcomes defined for actions in that state.
 *
 * Primary outcomes for specific state, action pairs are not currently
 * supported.
 */
class CustomReduction : public ReducedTransition
{
private:
    mlcore::Problem* problem_;

    std::unordered_map< mlcore::Action*, std::vector<bool> >
    primaryIndicatorsActions;

    std::unordered_map< mlcore::State*, std::vector<bool> >
    primaryIndicatorsStates;

public:
    CustomReduction(mlcore::Problem* problem) : problem_(problem) {

        for (mlcore::Action* action : problem->actions()) {
            primaryIndicatorsActions[action] = std::vector<bool> ();
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
     * Sets the primary outcomes indicators for the given action according to
     * the given vector of booleans.
     */
    virtual void setPrimaryForState(mlcore::State* s,
                                    std::vector<bool> primaryIndicators)
    {
        primaryIndicatorsStates[s] = primaryIndicators;
    }

    /**
     * Overrides method from ReducedTransition.
     */
    virtual void setPrimary(mlcore::State* s,
                            mlcore::Action *a,
                            std::vector<bool>& primaryIndicators) const
    {
        if (primaryIndicatorsStates.count(s)) {
            primaryIndicators = primaryIndicatorsStates.at(s);
            return;
        }
        const std::vector<bool>& indicators = primaryIndicatorsActions.at(a);
        assert(problem_->transition(s, a).size() == indicators.size());
        primaryIndicators = indicators;
    }
};

}
#endif // MDPLIB_CUSTOMREDUCTION_H
