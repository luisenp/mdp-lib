#include <cassert>

#include "../../../include/domains/ctp/CTPProblem.h"
#include "../../../include/domains/ctp/CTPAction.h"


CTPProblem::CTPProblem(Graph& roads, std::vector< std::vector <double> >& probs,
                       int start, int goal) : roads_(roads), probs_(probs), goal_(goal)
{
    s0 = new CTPState(this, start);
    absorbing_ = new CTPState(this, -1);
    this->addState(s0);
    this->addState(absorbing_);
    int n = roads_.numVertices();
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            actions_.push_back(new CTPAction(i, j));
        }
    }
    ((CTPState*) s0)->frontier().insert(start);
}

bool CTPProblem::goal(State* s) const
{
    CTPState* state = (CTPState*) s;
    return goal_ == state->location() ;
}

std::list<Successor> CTPProblem::transition(State* s, Action* a)
{
    assert(applicable(s, a));

    std::list<Successor> successors;
    if (s == absorbing_) {
        successors.push_front(Successor(s, Rational(1)));
        return successors;
    }
    if (goal(s)) {
        successors.push_front(Successor(absorbing_, Rational(1)));
        return successors;
    }

    CTPState* ctps = (CTPState *) s;
    CTPAction* ctpa = (CTPAction *) a;
    int from = ctpa->from();
    int to = ctpa->to();

    assert(ctps->frontier().find(to) != ctps->frontier().end());

    std::vector<int> neighbors;
    for (std::pair<int, double> entry : roads_.neighbors(to))
        neighbors.push_back(entry.first);
    int nadj = neighbors.size();

    for (int i = 0; i < (1 << nadj); i++) {
        CTPState* next = new CTPState(*ctps);
        for (int j = 0; j < nadj; j++) {
            unsigned char st = (i & 1<<j) ? ctp::open : ctp::blocked;
            next->setStatus(to, neighbors[j], st);
            next->frontier().insert(neighbors[j]);
            next->frontier().erase(to);
        }
    }

    return successors;
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
    if (ctps->location() != ctpa->from())
        return false;
    if (ctpa->to() == ctpa->from())
        return true;
    return ( ctps->frontier().find(ctpa->to())  != ctps->frontier().end() );
}
