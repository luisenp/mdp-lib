#ifndef GRIDWORLDSTATE_H
#define GRIDWORLDSTATE_H

#include <iostream>

#include "../../state.h"

class GridWorldState : public State
{
    private:
        int x_;
        int y_;

    public:
        GridWorldState() : x_(-1), y_(-1) {}

        GridWorldState(int x, int y) : x_(x), y_(y) {}

        GridWorldState(const GridWorldState & gws) : x_(gws.x_), y_(gws.y_) {}

    virtual State &operator=(const State & rhs)
    {
        if (this == &rhs)
            return *this;

        GridWorldState *gws = (GridWorldState *)  & rhs;
        x_ =  gws->x_;
        y_=  gws->y_;
        return *this;
    }

    virtual bool operator==(const State & rhs) const
    {
        GridWorldState *gws = (GridWorldState *)  & rhs;
        return x_ == gws->x_ && y_ == gws->y_;
    }

    friend std::ostream& operator<<(std::ostream& os, GridWorldState& gws);

    virtual int hash_value() const
    {
        return x_ + 31*y_;
    }

    int x() const
    {
        return x_;
    }

    int y() const
    {
        return y_;
    }
};

std::ostream& operator<<(std::ostream& os, GridWorldState& gws)
{
    os << "(" << gws.x_ << "," << gws.y_ << ")";
    return os;
}
#endif // GRIDWORLDSTATE_H
