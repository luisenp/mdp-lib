#ifndef MDPLIB_CTPACTION_H
#define MDPLIB_CTPACTION_H

#include "../../Action.h"

class CTPAction : public mlcore::Action
{
private:
    int from_;
    int to_;

    virtual std::ostream& print(std::ostream& os) const;

public:
    CTPAction(int from, int to) : from_(from), to_(to) {}

    int from()
    {
        return from_;
    }

    int to()
    {
        return to_;
    }

    /**
     * Overriding method from Action.
     */
    virtual mlcore::Action& operator=(const mlcore::Action& rhs)
    {
        if (this == &rhs)
            return *this;

        CTPAction* action = (CTPAction*)  & rhs;
        from_ =  action->from_;
        to_ =  action->to_;
        return *this;
    }

    /**
     * Overriding method from Action.
     */
    virtual int hashValue() const;
};

#endif // MDPLIB_CTPACTION_H
