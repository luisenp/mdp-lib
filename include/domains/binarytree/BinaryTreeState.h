#ifndef MDPLIB_BINARYTREESTATE_H
#define MDPLIB_BINARYTREESTATE_H

#include "../../state.h"


class BinaryTreeState : public State
{
private:
    int level_;
    int index_;

    virtual std::ostream& print(std::ostream& os) const;

public:
    BinaryTreeState();

    BinaryTreeState(int level, int index) : level_(level), index_(index) {}

    virtual ~BinaryTreeState() {}

    int level() { return level_; }

    int index() { return index_; }

    /**
     * Overrides method from State.
     */
    virtual State& operator=(const State& rhs)
    {
        if (this == &rhs)
            return *this;

        BinaryTreeState* state = (BinaryTreeState*)  & rhs;
        level_ =  state->level_;
        index_ =  state->index_;
        return *this;
    }

    /**
     * Overrides method from State.
     */
    virtual bool operator==(const State& rhs) const
    {
        BinaryTreeState* state = (BinaryTreeState*)  & rhs;
        return level_ == state->level_ && index_ == state->index_;
    }

    /**
     * Overrides method from State.
     */
    virtual bool equals(State* other) const;

    /**
     * Overrides method from State.
     */
    virtual int hashValue() const;

};

#endif // MDPLIB_BINARYTREESTATE_H
