#ifndef MDPLIB_SAILINGACTION_H
#define MDPLIB_SAILINGACTION_H

#include "../../action.h"


class SailingAction : public mlcore::Action
{
private:
    short dir_;

    virtual std::ostream& print(std::ostream& os) const
    {
        os << "Sailing Action: " << dir_;
        return os;
    }

public:
    SailingAction(short dir) : dir_(dir) {}

    virtual ~SailingAction() {}

    short dir() { return dir_; }

    /**
     * Overriding method from Action.
     */
    virtual mlcore::Action& operator=(const mlcore::Action& rhs)
    {
        if (this == &rhs)
            return *this;

        SailingAction* action = (SailingAction*)  & rhs;
        dir_ =  action->dir_;
        return *this;
    }

    /**
     * Overriding method from Action.
     */
    virtual int hashValue() const
    {
        return (int) dir_;
    }
};

#endif // MDPLIB_SAILINGACTION_H
