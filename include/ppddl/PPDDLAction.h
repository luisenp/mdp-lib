#ifndef MDPLIB_PPDDLACTION_H
#define MDPLIB_PPDDLACTION_H

#include "mini-gpt/states.h"
#include "mini-gpt/actions.h"
#include "mini-gpt/problems.h"

#include "../Action.h"

namespace mlppddl
{

class Action : public mlcore::Action
{
private:
    int index_;
    const action_t* pAction_;

    virtual std::ostream& print(std::ostream& os) const;

public:
    Action(const action_t* pAction, int index) :
        pAction_(pAction), index_(index) {}

    const action_t* pAction() { return pAction_; }

    /**
     * Overriding method from Action.
     */
    virtual mlcore::Action& operator=(const mlcore::Action& rhs)
    {
        if (this == &rhs)
            return *this;

        Action* action = (Action*)  & rhs;
        pAction_ = action->pAction_;
        index_ = action->index_;
        return *this;
    }

    /**
     * Overriding method from Action.
     */
    virtual int hashValue() const
    {
        return index_;
    }
};

}

#endif // MDPLIB_PPDDLACTION_H
