#include <cassert>

#include "../../../include/domains/ctp/CTPProblem.h"
#include "../../../include/domains/ctp/CTPAction.h"

#include "../../../include/util/general.h"

#include <unistd.h>

CTPProblem::CTPProblem(Graph& roads, std::vector< std::vector <double> >& probs,
                       int start, int goal)
                       : roads_(roads), probs_(probs), start_(start), goal_(goal)
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
    actions_.push_back(new CTPAction(-1,-1));
}

bool CTPProblem::goal(State* s) const
{
    CTPState* ctps = (CTPState*) s;
    if (ctps->badWeather())
        return true;
    return goal_ == ctps->location() ;
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

    std::vector<int> neighbors;
    for (std::pair<int, double> entry : roads_.neighbors(to)) {
        /* No need to go back to states that have already been explored */
        if (ctps->explored().find(entry.first) == ctps->explored().end())
            neighbors.push_back(entry.first);
    }
    int nadj = neighbors.size();
    for (int i = 0; i < (1 << nadj); i++) {
        CTPState* next = new CTPState(*ctps);
        next->setLocation(to);
        double p = 1.0;
        /* Updating adjacent roads */
        for (int j = 0; j < nadj; j++) {
            assert(ctps->status()[to][neighbors[j]] == ctp::UNKNOWN);
            unsigned char st = (i & (1<<j)) ? ctp::OPEN : ctp::BLOCKED;
            p *= (st == ctp::BLOCKED) ? 1.0 - probs_[to][neighbors[j]] : probs_[to][neighbors[j]];
            next->setStatus(to, neighbors[j], st);
            next->setStatus(neighbors[j], to, st);
        }
        next->explored().insert(to);
        successors.push_back(Successor(this->addState(next), Rational(p)));
    }
    return successors;
}

Rational CTPProblem::cost(State* s, Action* a) const
{
    assert(applicable(s, a));
    if (s == absorbing_ || goal(s)) {
        return Rational(0);
    }
    CTPState* ctps = (CTPState *) s;
    CTPAction* ctpa = (CTPAction *) a;
    return Rational(ctps->distanceOpen(ctpa->to()));
}

bool CTPProblem::applicable(State* s, Action* a) const
{
    CTPState* ctps = (CTPState *) s;
    CTPAction* ctpa = (CTPAction *) a;
    if (s == absorbing_ || goal(s))
        return ctpa->from() == -1;
    if (ctps->location() != ctpa->from())
        return false;
    if (!ctps->reachable(ctpa->to()))
        return false;

    /* Checking if the state was previously explored. If so, no need to explore it again */
    return ( ctps->explored().find(ctpa->to())  == ctps->explored().end() );
}
