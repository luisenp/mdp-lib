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
     * Constructs a GridWorldState representing grid position (x,y) on the problem
     * given as a first parameter.
     */
    GridWorldState(Problem* problem, int x, int y);

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
