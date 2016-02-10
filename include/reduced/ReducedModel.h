#ifndef MDPLIB_REDUCEDMODEL_H
#define MDPLIB_REDUCEDMODEL_H

#include <list>

#include "../Action.h"
#include "../Problem.h"
#include "../State.h"

#include "ReducedState.h"
#include "ReducedTransitionConfig.h"

namespace mlreduced
{

class ReducedModel : public mlcore::Problem
{
protected:
    /**
     * The problem for which the reduced model is constructed.
     */
    mlcore::Problem *originalProblem_;

    /**
     * An object describing the outcomes to be considered primary for
     * each state, action pair.
     */
    ReducedTransitionConfig *config_;

    /**
     * The bound on the number of exceptions.
     */
    int k_;

public:
    ReducedModel(mlcore::Problem *originalProblem,
                 ReducedTransitionConfig *config) :
        originalProblem_(originalProblem), config_(config)
    {
        s0 = new ReducedState(originalProblem_->initialState(), 0);
        this->addState(s0);
    }

    virtual ~ReducedModel() {}

    /**
     * Implements a reduced transition model for this problem according to
     * the stored configuration.
     */
    virtual std::list<mlcore::Successor>
    transition(mlcore::State* s, mlcore::Action *a);

    /**
     * Overrides method from Problem.
     */
    virtual bool goal(mlcore::State* s) const
    {
        return originalProblem_->goal(s);
    }

    /**
     * Overrides method from Problem.
     */
    virtual double cost(mlcore::State* s, mlcore::Action* a) const
    {
        return originalProblem_->cost(s, a);
    }

    /**
     * Overrides method from Problem.
     */
    virtual bool applicable(mlcore::State* s, mlcore::Action* a) const
    {
        return originalProblem_->applicable(s, a);
    }
};

}


#endif // MDPLIB_REDUCEDMODEL_H
