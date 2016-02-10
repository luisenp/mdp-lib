#ifndef MPDLIB_GRIDWORLDACTION_H
#define MPDLIB_GRIDWORLDACTION_H

#include "../../Action.h"

class GridWorldAction : public mlcore::Action
{
private:
    unsigned char dir_;
    virtual std::ostream& print(std::ostream& os) const;

public:
    GridWorldAction() : dir_(-1) {}

    GridWorldAction(const unsigned char dir) : dir_(dir) {}

    /**
     * Overriding method from Action.
     */
    virtual mlcore::Action& operator=(const mlcore::Action& rhs)
    {
        if (this == &rhs)
            return *this;

        GridWorldAction* action = (GridWorldAction*)  & rhs;
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

    unsigned char dir() const
    {
        return dir_;
    }
};

#endif // MPDLIB_GRIDWORLDACTION_H
