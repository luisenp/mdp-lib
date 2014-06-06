#ifndef MDPLIB_BINARYTREEPROBLEM_H
#define MDPLIB_BINARYTREEPROBLEM_H

#include "../../problem.h"


class BinaryTreeProblem : public Problem
{
private:
    int levels_;
    State* absorbing_;

public:
    BinaryTreeProblem();

    BinaryTreeProblem(int levels);

    virtual ~BinaryTreeProblem() {}

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

#endif // MDPLIB_BINARYTREEPROBLEM_H
