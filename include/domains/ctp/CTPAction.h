#ifndef MDPLIB_CTPACTION_H
#define MDPLIB_CTPACTION_H

#include "../../action.h"

class CTPAction : public Action
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
    virtual int hashValue() const;
};

#endif // MDPLIB_CTPACTION_H
