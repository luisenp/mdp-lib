#ifndef MDPLIB_RAWFILELEXIACTION_H
#define MDPLIB_RAWFILELEXIACTION_H

#include "../../action.h"

class RawFileLexiAction : public mlcore::Action
{
private:
    int id_;

    virtual std::ostream& print(std::ostream& os) const
    {
        os << "Raw File Action: " << id_;
        return os;
    }

public:
    RawFileLexiAction(int id) : id_(id) { }

    virtual ~RawFileLexiAction() {}

    int id() { return id_; }

    /**
     * Overriding method from Action.
     */
    virtual mlcore::Action& operator=(const mlcore::Action& rhs)
    {
        if (this == &rhs)
            return *this;

        RawFileLexiAction* action = (RawFileLexiAction*)  & rhs;
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

#endif // MDPLIB_RAWFILELEXIACTION_H
