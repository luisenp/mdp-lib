#ifndef MDPLIB_WRAPPERPROBLEM_H
#define MDPLIB_WRAPPERPROBLEM_H

#include "../problem.h"
#include "DummyState.h"

class WrapperProblem : mlcore::Problem
{
private:

    DummyState* dummyState_;

    mlcore::Problem* problem_;
    mlcore::Action* dummyAction_;

public:

    WrapperProblem(mlcore::Problem* problem) : problem_(problem)
    {
        dummyState_ = new DummyState();
    }

    virtual ~WrapperProblem()
    {
        delete dummyState_;
    }

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
