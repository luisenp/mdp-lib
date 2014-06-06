#include "../../../include/domains/binarytree/BinaryTreeProblem.h"
#include "../../../include/domains/binarytree/BinaryTreeState.h"
#include "../../../include/domains/binarytree/BinaryTreeAction.h"

BinaryTreeProblem::BinaryTreeProblem(int levels) : levels_(levels)
{
    s0 = new BinaryTreeState(0, 0);
    absorbing_ = new BinaryTreeState(levels, 0);
    addState(s0);
    addState(absorbing_);
    actions_.push_front(new BinaryTreeAction(true));
    actions_.push_front(new BinaryTreeAction(false));
}

bool BinaryTreeProblem::goal(State* s) const
{
    BinaryTreeState* bts = (BinaryTreeState*) s;
    return bts->level() == (levels_ - 1);
}

std::list<Successor> BinaryTreeProblem::transition(State* s, Action* a)
{
    std::list<Successor> successors;
    if (s == absorbing_ || goal(s)) {
        successors.push_front(Successor(absorbing_, Rational(1)));
        return successors;
    }
    BinaryTreeState* bts = (BinaryTreeState *) s;
    BinaryTreeAction* bta = (BinaryTreeAction *) a;
    State* next = new BinaryTreeState(bts->level() + 1, 2*bts->index() + ( 1 - bta->left()));
    next = this->addState(next);
    successors.push_front(Successor(next, Rational(1)));
    return successors;
}

Rational BinaryTreeProblem::cost(State* s, Action* a) const
{
    BinaryTreeState* bts = (BinaryTreeState *) s;
    BinaryTreeAction* bta = (BinaryTreeAction *) a;
    if (bts->level() != levels_ - 2)
        return Rational(0);
    if (bts->index() != 0)
        return Rational(1);
    return bta->left() ? Rational(0) : Rational(1);
}

bool BinaryTreeProblem::applicable(State* s, Action* a) const
{
    return true;
}
