#ifndef MDPLIB_REDUCEDMODEL_H
#define MDPLIB_REDUCEDMODEL_H

#include "../action.h"
#include "../problem.h"
#include "../state.h"

#include "ReducedState.h"

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
     * The bound on the number of exceptions.
     */
    int k;

public:
    ReducedModel(mlcore::Problem *originalProblem) :
        originalProblem_ = originalProblem
    {
        s0 = new ReducedState(originalProblem_->initialState(), 0);
        this->addState(s0);
    }

    virtual ~ReducedModel() {}

    /**
     * Overrides method from Problem.
     */
    virtual bool goal(mlcore::State* s) const;
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
