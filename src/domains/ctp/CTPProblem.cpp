#include "../../../include/domains/ctp/CTPProblem.h"

bool CTPProblem::goal(State* s) const
{
    return false;
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
