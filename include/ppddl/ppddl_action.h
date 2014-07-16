#ifndef MDPLIB_PPDDLACTION_H
#define MDPLIB_PPDDLACTION_H

#include "../action.h"
#include "mini-gpt/states.h"

namespace mlppddl
{

class Action : public mlcore::Action
{
private:
    int index_;
    const action_t* pAction_;

    virtual std::ostream& print(std::ostream& os) const;

public:
    Action(const action_t* pAction, int index) : index_(index), pAction_(pAction) {}

    const action_t* pAction() { return pAction_; }

    int index() { return index_; }

    /**
     * Overriding method from Action.
     */
    virtual mlcore::Action& operator=(const mlcore::Action& rhs)
    {
        if (this == &rhs)
            return *this;

        Action* action = (Action*)  & rhs;
        pAction_ =  action->pAction_;
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
