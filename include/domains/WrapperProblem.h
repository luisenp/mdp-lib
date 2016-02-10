#ifndef MDPLIB_WRAPPERPROBLEM_H
#define MDPLIB_WRAPPERPROBLEM_H

#include "../Problem.h"
#include "DummyState.h"

/**
 * This class wraps a mlcore::Problem instance and adds to it a DummyState whose
 * successors can be defined externally.
 */
class WrapperProblem : public mlcore::Problem
{
private:

    DummyState* dummyState_;

    mlcore::Problem* problem_;
    mlcore::Action* dummyAction_;

public:

    WrapperProblem(mlcore::Problem* problem) : problem_(problem)
    {
        dummyState_ = new DummyState();
        s0 = problem_->initialState();
        actions_ = problem->actions();
        heuristic_ = problem->heuristic();
    }

    virtual ~WrapperProblem() { }

    DummyState* dummyState() { return dummyState_; }

    void setDummyAction(mlcore::Action* action) { dummyAction_ = action; }

    /**
     * Overrides method from Problem.
     */
    virtual bool goal(mlcore::State* s) const;

    /**
     * Overrides method from Problem.
     */
    virtual std::list<mlcore::Successor> transition(mlcore::State* s, mlcore::Action* a);

    /**
     * Overrides method from Problem.
     */
    virtual double cost(mlcore::State* s, mlcore::Action* a) const;

    /**
     * Overrides method from Problem.
     */
    virtual bool applicable(mlcore::State* s, mlcore::Action* a) const;
};

#endif // MDPLIB_WRAPPERPROBLEM_H
