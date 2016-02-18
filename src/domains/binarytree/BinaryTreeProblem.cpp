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

bool BinaryTreeProblem::goal(mlcore::State* s) const
{
    BinaryTreeState* bts = static_cast<BinaryTreeState*> (s);
    return bts->level() == (levels_ - 1);
}

std::list<mlcore::Successor>
BinaryTreeProblem::transition(mlcore::State* s, mlcore::Action* a)
{
    std::list<mlcore::Successor> successors;
    if (s == absorbing_ || goal(s)) {
        successors.push_front(mlcore::Successor(absorbing_, 1.0));
        return successors;
    }
    BinaryTreeState* bts = static_cast<BinaryTreeState*> (s);
    BinaryTreeAction* bta = static_cast<BinaryTreeAction*> (a);
    mlcore::State* next =
        new BinaryTreeState(bts->level() + 1,
                            2*bts->index() + ( 1 - bta->left()));
    next = this->addState(next);
    successors.push_front(mlcore::Successor(next, 1.0));
    return successors;
}

double BinaryTreeProblem::cost(mlcore::State* s, mlcore::Action* a) const
{
    BinaryTreeState* bts = static_cast<BinaryTreeState*> (s);
    BinaryTreeAction* bta = static_cast<BinaryTreeAction*> (a);
    if (bts->level() != levels_ - 2)
        return 0.0;
    if (bts->index() != 0)
        return 1.0;
    return bta->left() ? 0.0 : 1.0;
}

bool BinaryTreeProblem::applicable(mlcore::State* s, mlcore::Action* a) const
{
    return true;
}
