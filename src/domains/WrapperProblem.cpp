#include "../../include/domains/WrapperProblem.h"

bool WrapperProblem::goal(mlcore::State* s) const
{
    if (s == dummyState_)
        return false;
    if (s == absorbing_)
        return true;
    return problem_->goal(s);
}


std::list<mlcore::Successor>
WrapperProblem::transition(mlcore::State* s, mlcore::Action* a)
{
    if (s == absorbing_ ||
            (overrideGoals_ != nullptr && overrideGoals_->count(s) > 0)) {
        std::list<mlcore::Successor> successors;
        successors.push_back(mlcore::Successor(absorbing_, 1.0));
        return successors;
    }
    if (s == dummyState_)
        return ((DummyState *) s)->successors();
    return problem_->transition(s, a);
}


double WrapperProblem::cost(mlcore::State* s, mlcore::Action* a) const
{
    if (s == dummyState_ || s == absorbing_)
        return 0.0;
    if (overrideGoals_ != nullptr &&
            !overrideGoals_->empty() && overrideGoals_->count(s) > 0)
        return s->cost();
    return problem_->cost(s, a);
}


bool WrapperProblem::applicable(mlcore::State* s, mlcore::Action* a) const
{
    if (s == dummyState_ || s == absorbing_)
        return a == dummyAction_;
    if (overrideGoals_ != nullptr &&
            !overrideGoals_->empty() && overrideGoals_->count(s) > 0) {
        if (s->bestAction() != nullptr) {
            return (a == s->bestAction());
        }
    }
    return problem_->applicable(s, a);
}


mlcore::StateSet& WrapperProblem::states()
{
    if (overrideStates_ != nullptr && !overrideStates_->empty())
        return *overrideStates_;
    return problem_->states();
}
