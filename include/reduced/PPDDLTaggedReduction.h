#ifndef PPDDLTAGGEDREDUCTION_H
#define PPDDLTAGGEDREDUCTION_H

#include <list>
#include <vector>

#include "../ppddl/PPDDLProblem.h"
#include "../ppddl/PPDDLState.h"

#include "../State.h"

#include "ReducedTransition.h"


namespace mlreduced
{

//TODO: Might need to delete this class.
class PPDDLTaggedReduction : public ReducedTransition
{
private:
    mlcore::Problem* problem_;

public:
    PPDDLTaggedReduction(mlcore::Problem* problem) : problem_(problem) { }

    virtual ~PPDDLTaggedReduction() { }

    /**
     * Overrides method from ReducedTransition.
     */
    virtual
    getPrimaryIndicators(mlcore::State* s,
                         mlcore::Action *a,
                         std::vector<bool>& primaryIndicator) const
    {
//        mlppddl::PPDDLProblem* ppddlProblem = (mlppddl::PPDDLProblem *) problem_;
//        mlppddl::PPDDLState* ppddlState = (mlppddl::PPDDLState *) s;
//        state_t* internalPPDDLState = ppddlState->pState();
//
//        std::list<mlcore::Successor> successors = problem_->transition(s, a);
//        std::vector<bool> primaryValues(successors.size(), false);
//        for (mlcore::Successor successor : successors) {
//            dprint1(successor.su_state, successor.su_prob);
//        }
    }

    /**
     * Overrides method from ReducedTransition.
     */
    virtual void
    getIsCounterIncrementer(mlcore::State* s,
                            mlcore::Action *a,
                            std::vector<bool>& isCounterIncrementer) const
    {

    }
};

}
#endif // PPDDLTAGGEDREDUCTION_H


