#ifndef MDPLIB_CTPPROBLEM_H
#define MDPLIB_CTPPROBLEM_H

#include "../../problem.h"


class CTPProblem : public Problem
{
private:
public:
    CTPProblem();
    virtual ~CTPProblem();

    virtual bool goal(State* s) const;
    virtual std::list<Successor> transition(State* s, Action* a);
    virtual Rational cost(State* s, Action* a) const;
    virtual bool applicable(State* s, Action* a) const;
    virtual void generateAll();
};

#endif // MDPLIB_CTPPROBLEM_H
