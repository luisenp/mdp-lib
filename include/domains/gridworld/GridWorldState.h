#ifndef MPDLIB_GRIDWORLDSTATE_H
#define MPDLIB_GRIDWORLDSTATE_H

#include <iostream>

#include "../../state.h"

class GridWorldState : public State
{
private:
    int x_;
    int y_;

    virtual std::ostream& print(std::ostream& os) const;

public:
    GridWorldState() : x_(-1), y_(-1) {}

    GridWorldState(int x, int y) : x_(x), y_(y) {}

    GridWorldState(const GridWorldState& gws) : x_(gws.x_), y_(gws.y_) {}

    virtual State &operator=(const State& rhs)
    {
        if (this == &rhs)
            return *this;

        GridWorldState *gws = (GridWorldState *)  & rhs;
        x_ =  gws->x_;
        y_=  gws->y_;
        return *this;
    }

    virtual bool operator==(const State& rhs) const
    {
        GridWorldState *gws = (GridWorldState *)  & rhs;
        return x_ == gws->x_ && y_ == gws->y_;
    }

    virtual int hash_value() const;

    int x() const;

    int y() const;
};

#endif // MDPLIB_GRIDWORLDSTATE_H
