#ifndef MDPLIB_BINARYTREEPROBLEM_H
#define MDPLIB_BINARYTREEPROBLEM_H

#include "../../Problem.h"


class BinaryTreeProblem : public mlcore::Problem
{
private:
    int levels_;
    mlcore::State* absorbing_;

public:
    BinaryTreeProblem();

    BinaryTreeProblem(int levels);

    virtual ~BinaryTreeProblem() {}

    /**
     * Overrides method from mlcore::Problem.
     */
    virtual bool goal(mlcore::State* s) const;

    /**
     * Overrides method from mlcore::Problem.
     */
    virtual std::list<mlcore::Successor>
    transition(mlcore::State* s, mlcore::Action* a);

    /**
     * Overrides method from mlcore::Problem.
     */
    virtual double cost(mlcore::State* s, mlcore::Action* a) const;

    /**
     * Overrides method from mlcore::Problem.
     */
    virtual bool applicable(mlcore::State* s, mlcore::Action* a) const;
};

#endif // MDPLIB_BINARYTREEPROBLEM_H
