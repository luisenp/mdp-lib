#ifndef MDPLIB_RAWFILEMOACTION_H
#define MDPLIB_RAWFILEMOACTION_H

#include "../../action.h"

class RawFileMOAction : public mlcore::Action
{
private:
    int id_;

    virtual std::ostream& print(std::ostream& os) const
    {
        os << "Raw File Action: " << id_;
        return os;
    }

public:
    RawFileMOAction(int id) : id_(id) { }

    virtual ~RawFileMOAction() {}

    int id() { return id_; }

    /**
     * Overriding method from Action.
     */
    virtual mlcore::Action& operator=(const mlcore::Action& rhs)
    {
        if (this == &rhs)
            return *this;

        RawFileMOAction* action = (RawFileMOAction*)  & rhs;
        id_ =  action->id_;
        return *this;
    }

    /**
     * Overriding method from Action.
     */
    virtual int hashValue() const
    {
        return id_;
    }
};

#endif // MDPLIB_RAWFILEMOACTION_H
