#include <cassert>

#include "../../../include/domains/ctp/CTPProblem.h"
#include "../../../include/domains/ctp/CTPAction.h"


CTPProblem::CTPProblem(Graph& roads, int start, int goal) : roads_(roads), goal_(goal)
{
    initial_ = new CTPState(this, start);
    absorbing_ = new CTPState(this, -1);
    this->addState(initial_);
    this->addState(absorbing_);
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
    assert(applicable(s, a));
    CTPState* ctps = (CTPState *) s;
    CTPAction* ctpa = (CTPAction *) a;
    std::vector<double> distances = dijkstra(roads_, ctps->location());
    return Rational(distances[ctpa->to()]);
}

bool CTPProblem::applicable(State* s, Action* a) const
{
    CTPState* ctps = (CTPState *) s;
    CTPAction* ctpa = (CTPAction *) a;
    return ctps->location() == ctpa->from();
}
