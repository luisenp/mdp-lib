#ifndef GRIDWORLDSTATE_H
#define GRIDWORLDSTATE_H

#include "../../state.h"

class GridWorldState : public State
{
    private:
        int x_;
        int y_;

    public:
        GridWorldState();
        GridWorldState(int x, int y);
        ~GridWorldState();

    virtual State *operator=(const State *other);

    virtual bool operator==(const State *other)
    {
        GridWorldState *gws = (GridWorldState *)  other;
        return x_ == gws->x_ && y_ == gws->y_;
    }

    virtual std::string to_string()
    {
        return "";
    }

    virtual int hash_value()
    {
        return x_ + 31*y_;
    }
};

#endif // GRIDWORLDSTATE_H
