#ifndef MDPLIB_BINARYTREESTATE_H
#define MDPLIB_BINARYTREESTATE_H

#include "../../State.h"


class BinaryTreeState : public mlcore::State
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
     * Overrides method from mlcore::State.
     */
    virtual mlcore::State& operator=(const mlcore::State& rhs)
    {
        if (this == &rhs)
            return *this;

        BinaryTreeState* state = (BinaryTreeState*)  & rhs;
        level_ =  state->level_;
        index_ =  state->index_;
        return *this;
    }

    /**
     * Overrides method from mlcore::State.
     */
    virtual bool operator==(const mlcore::State& rhs) const
    {
        BinaryTreeState* state = (BinaryTreeState*)  & rhs;
        return level_ == state->level_ && index_ == state->index_;
    }

    /**
     * Overrides method from mlcore::State.
     */
    virtual bool equals(mlcore::State* other) const;

    /**
     * Overrides method from mlcore::State.
     */
    virtual int hashValue() const;

};

#endif // MDPLIB_BINARYTREESTATE_H
