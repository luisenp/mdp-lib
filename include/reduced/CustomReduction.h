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
    primaryIndicatorsActions_;

    std::unordered_map< mlcore::State*, std::vector<bool> >
    primaryIndicatorStates_;

public:
    CustomReduction(mlcore::Problem* problem) : problem_(problem) {

        for (mlcore::Action* action : problem->actions()) {
            primaryIndicatorsActions_[action] = std::vector<bool> ();
        }
    }

    CustomReduction(CustomReduction* reduction)
    {
        problem_ = reduction->problem_;
        for (std::pair< mlcore::Action*, std::vector<bool> > entry :
                reduction->primaryIndicatorsActions_) {
            primaryIndicatorsActions_[entry.first] = entry.second;
        }
        for (std::pair< mlcore::State*, std::vector<bool> > entry :
                reduction->primaryIndicatorStates_) {
            primaryIndicatorStates_[entry.first] = entry.second;
        }
    }

    virtual ~CustomReduction() {}

    std::unordered_map< mlcore::Action*, std::vector<bool> > &
    primaryIndicatorsActions()
    {
        return primaryIndicatorsActions_;
    }

    /**
     * Sets the primary outcomes indicators for the given action according to
     * the given vector of booleans.
     */
    virtual void setPrimaryForAction(mlcore::Action* a,
                                     std::vector<bool> primaryIndicators)
    {
        assert(primaryIndicatorsActions_.count(a) > 0);
        primaryIndicatorsActions_[a] = primaryIndicators;
    }

    /**
     * Sets the primary outcomes indicators for the given action according to
     * the given vector of booleans.
     */
    virtual void setPrimaryForState(mlcore::State* s,
                                    std::vector<bool> primaryIndicators)
    {
        primaryIndicatorStates_[s] = primaryIndicators;
    }

    /**
     * Overrides method from ReducedTransition.
     */
    virtual void setPrimary(mlcore::State* s,
                            mlcore::Action *a,
                            std::vector<bool>& primaryIndicators) const
    {
        if (primaryIndicatorStates_.count(s)) {
            primaryIndicators = primaryIndicatorStates_.at(s);
            return;
        }
        if (!problem_->applicable(s, a))
            return;
        const std::vector<bool>& indicators = primaryIndicatorsActions_.at(a);
        assert(problem_->transition(s, a).size() == indicators.size());
        primaryIndicators = indicators;
    }
};

}
#endif // MDPLIB_CUSTOMREDUCTION_H
