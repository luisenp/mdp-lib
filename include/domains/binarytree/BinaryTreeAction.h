#ifndef MDPLIB_BINARYTREEACTION_H
#define MDPLIB_BINARYTREEACTION_H

#include "../../Action.h"


class BinaryTreeAction : public mlcore::Action
{
private:
    bool left_;

    virtual std::ostream& print(std::ostream& os) const
    {
        os << "BTAction: " << (left_ ? "left" : "right");
        return os;
    }

public:
    BinaryTreeAction();

    BinaryTreeAction(bool left) : left_(left) { }

    virtual ~BinaryTreeAction() {}

    bool left() { return left_; }

    /**
     * Overriding method from mlcore::Action.
     */
    virtual mlcore::Action& operator=(const mlcore::Action& rhs)
    {
        if (this == &rhs)
            return *this;

        BinaryTreeAction* action = (BinaryTreeAction*)  & rhs;
        left_ =  action->left_;
        return *this;
    }

    /**
     * Overriding method from mlcore::Action.
     */
    virtual int hashValue() const
    {
        return left_;
    }
};

#endif // MDPLIB_BINARYTREEACTION_H
