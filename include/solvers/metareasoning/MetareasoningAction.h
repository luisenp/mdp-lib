#ifndef METAREASONINGACTION_H
#define METAREASONINGACTION_H

#include "../../action.h"

class MetareasoningAction : public mlcore::Action
{
private:
    bool isNOP_;

    virtual std::ostream& print(std::ostream& os) const;

public:
    MetareasoningAction() : isNOP_(false) {}

    MetareasoningAction(const bool isNOP) : isNOP_(isNOP) {}

    /**
     * Overriding method from Action.
     */
    virtual mlcore::Action& operator=(const mlcore::Action& rhs)
    {
        if (this == &rhs)
            return *this;

        MetareasoningAction* action = (MetareasoningAction*)  & rhs;
        isNOP_ =  action->isNOP_;
        return *this;
    }

    /**
     * Overriding method from Action.
     */
    virtual int hashValue() const
    {
        return (int) isNOP_;
    }

    unsigned char isNOP() const
    {
        return isNOP_;
    }
};

#endif // METAREASONINGACTION_H
