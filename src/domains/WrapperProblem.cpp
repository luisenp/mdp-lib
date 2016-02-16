#include "../../include/domains/WrapperProblem.h"

bool WrapperProblem::goal(mlcore::State* s) const
{
    if (s == dummyState_)
        return false;
    if (!overrideGoals_.empty())
        return overrideGoals_.count(s) > 0;
    return problem_->goal(s);
}

std::list<mlcore::Successor> WrapperProblem::transition(mlcore::State* s, mlcore::Action* a)
{
    if (s == dummyState_)
        return ((DummyState *) s)->successors();
    return problem_->transition(s, a);
}

double WrapperProblem::cost(mlcore::State* s, mlcore::Action* a) const
{
    if (s == dummyState_)
        return 0.0;
    return problem_->cost(s, a);
}

bool WrapperProblem::applicable(mlcore::State* s, mlcore::Action* a) const
{
    if (s == dummyState_)
        return a == dummyAction_;
    return problem_->applicable(s, a);
}
