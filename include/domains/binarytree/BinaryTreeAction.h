#ifndef MDPLIB_BINARYTREEACTION_H
#define MDPLIB_BINARYTREEACTION_H

#include "../../action.h"


class BinaryTreeAction : public Action
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
     * Overriding method from Action.
     */
    virtual int hashValue() const
    {
        return left_;
    }
};

#endif // MDPLIB_BINARYTREEACTION_H
