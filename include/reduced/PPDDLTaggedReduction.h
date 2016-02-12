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

class PPDDLTaggedReduction : public ReducedTransition
{
private:
    mlcore::Problem* problem_;

public:
    PPDDLTaggedReduction(mlcore::Problem* problem) : problem_(problem) { }

    virtual ~PPDDLTaggedReduction() { }

    virtual
    std::vector<bool> isPrimary(mlcore::State* s, mlcore::Action *a) const
    {
        mlppddl::PPDDLProblem* ppddlProblem = (mlppddl::PPDDLProblem *) problem_;
        mlppddl::PPDDLState* ppddlState = (mlppddl::PPDDLState *) s;
        state_t* internalPPDDLState = ppddlState->pState();

        std::list<mlcore::Successor> successors = problem_->transition(s, a);
        std::vector<bool> primaryValues(successors.size(), false);
        for (mlcore::Successor successor : successors) {
            dprint1(successor.su_state, successor.su_prob);
        }
        return primaryValues;
    }
};

}
#endif // PPDDLTAGGEDREDUCTION_H


