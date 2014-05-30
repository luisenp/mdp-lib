#ifndef MDPLIB_CTPPROBLEM_H
#define MDPLIB_CTPPROBLEM_H

#include "../../problem.h"
#include "../../util/graph.h"
#include "../../domains/ctp/CTPState.h"

namespace ctp
{
    const int blocked = 0;
    const int open = 1;
    const int unknown = 2;
}

class CTPProblem : public Problem
{
private:
    int goal_;
    Graph& roads_;
    CTPState* initial_;

public:
    CTPProblem(Graph& roads, int start, int goal);
     virtual ~CTPProblem() {}

    Graph& roads()
    {
        return roads_;
    }

    virtual bool goal(State* s) const;
    virtual std::list<Successor> transition(State* s, Action* a);
    virtual Rational cost(State* s, Action* a) const;
    virtual bool applicable(State* s, Action* a) const;
    virtual void generateAll();
};

#endif // MDPLIB_CTPPROBLEM_H
