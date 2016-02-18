#ifndef MDPLIB_SAILINGACTION_H
#define MDPLIB_SAILINGACTION_H

#include "../../Action.h"


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

    short dir() const { return dir_; }

    /**
     * Overriding method from Action.
     */
    virtual mlcore::Action& operator=(const mlcore::Action& rhs)
    {
        if (this == &rhs)
            return *this;

        const SailingAction* action = static_cast<const SailingAction*> (&rhs);
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
