#ifndef MDPLIB_PROBLEM_H
#define MDPLIB_PROBLEM_H

#include <list>
#include "../include/state.h"
#include "../include/action.h"
#include "../include/util/rational.h"

/**
 * An abstract class for Stochastic Shortest Path Problem objects.
 */
class Problem
{
protected:
    State *s0;
    std::list<Action *> actions;

public:
    Problem();
    virtual ~Problem();
    Problem(const Problem& other);
    Problem& operator=(const Problem& other);

    /**
     * Returns true if state s is a goal.
     */
    virtual bool goal(const State *s) const =0;

    /**
     * Returns a list of all succcessors when action a is applied in state s.
     */
    virtual std::list<Successor> transition(State *s, Action *a) const =0;

    /**
     * Returns the cost of applying action a in state s.
     */
    virtual Rational cost(State *s, Action *a) const =0;

    /**
     * Returns true if action a can be applied in state s.
     */
    virtual bool applicable(State *s, Action *a) const =0;
};

#endif // MDPLIB_PROBLEM_H
