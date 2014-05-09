#ifndef MPDLIB_GRIDWORLDACTION_H
#define MPDLIB_GRIDWORLDACTION_H

#include "../../action.h"

class GridWorldAction : public Action
{
private:
    unsigned char dir_;
public:
    GridWorldAction() : dir_(-1) {}

    GridWorldAction(const unsigned char dir) : dir_(dir) {}

    virtual int hash_value() const
    {
        return (int) dir_;
    }

    unsigned char dir() const
    {
        return dir_;
    }
};

#endif // MPDLIB_GRIDWORLDACTION_H
