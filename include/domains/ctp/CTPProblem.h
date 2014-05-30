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

/**
 * A class implementing the Canadian Traveler Problem.
 */
class CTPProblem : public Problem
{
private:
    int goal_;
    Graph& roads_;
    CTPState* initial_;
    CTPState* absorbing_;

public:
    CTPProblem(Graph& roads, int start, int goal);
     virtual ~CTPProblem() {}

    Graph& roads()
    {
        return roads_;
    }

    /**
     * Overrides method from Problem.
     */
    virtual bool goal(State* s) const;

    /**
     * Overrides method from Problem.
     */
    virtual std::list<Successor> transition(State* s, Action* a);

    /**
     * Overrides method from Problem.
     */
    virtual Rational cost(State* s, Action* a) const;

    /**
     * Overrides method from Problem.
     */
    virtual bool applicable(State* s, Action* a) const;
};

#endif // MDPLIB_CTPPROBLEM_H
