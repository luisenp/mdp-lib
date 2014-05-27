#ifndef MPDLIB_GRIDWORLDSTATE_H
#define MPDLIB_GRIDWORLDSTATE_H

#include <iostream>

#include "../../state.h"

/**
 * A class representing a grid world state as described in AIAMA 3rd Edition.
 */
class GridWorldState : public State
{
private:
    int x_;
    int y_;

    virtual std::ostream& print(std::ostream& os) const;

public:
    /**
     * Default constructor.
     * It initializes the grid position of the resulting state to a
     * dummy value of (-1,-1).
     */
    GridWorldState() : x_(-1), y_(-1) {}

    /**
     * Constructs a GridWorldState representing grid position (x,y).
     */
    GridWorldState(int x, int y) : x_(x), y_(y) {}

    /**
     * Copy constructor. The resulting state represents the same position as the
     * state passed as parameter.
     */
    GridWorldState(const GridWorldState& gws) : x_(gws.x_), y_(gws.y_) {}

    virtual State& operator=(const State& rhs)
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
        GridWorldState *gws = (GridWorldState *)  &rhs;
        return x_ == gws->x_ && y_ == gws->y_;
    }

    virtual bool equals(State* other) const;
    virtual int hashValue() const;

    int x() const;

    int y() const;
};

#endif // MDPLIB_GRIDWORLDSTATE_H
