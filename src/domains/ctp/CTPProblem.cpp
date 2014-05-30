#include "../../../include/domains/ctp/CTPProblem.h"


CTPProblem::CTPProblem(Graph& roads, int start, int goal) : roads_(roads), goal_(goal)
{
    initial_ = new CTPState(this, start);
    absorbing_ = new CTPState(this, -1);
    this->addState(initial_);
}

bool CTPProblem::goal(State* s) const
{
    CTPState* state = (CTPState*) s;
    return goal_ == state->location() ;
}

std::list<Successor> CTPProblem::transition(State* s, Action* a)
{
    return std::list<Successor> ();
}

Rational CTPProblem::cost(State* s, Action* a) const
{
    return Rational(0);
}

bool CTPProblem::applicable(State* s, Action* a) const
{
    return true;
}

void CTPProblem::generateAll()
{
}
