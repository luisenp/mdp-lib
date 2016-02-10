#ifndef MDPLIB_MOGRIDWORLDSTATE_H
#define MDPLIB_MOGRIDWORLDSTATE_H

#include "../MObjState.h"

namespace mlmobj
{

/**
 * A class representing a lexicographical version of the grid world state as described
 * in AIAMA 3rd Edition.
 */
class MOGridWorldState : public MOState
{
private:
    int x_;
    int y_;

    virtual std::ostream& print(std::ostream& os) const;

public:

    /**
     * Constructs a MOGridWorldState representing grid position (x,y) on the problem
     * given as a first parameter.
     */
    MOGridWorldState(mlcore::Problem* problem, int x, int y);

    /**
     * Copy constructor. The resulting state represents the same position as the
     * state passed as parameter.
     */
    MOGridWorldState(const MOGridWorldState& gws) : x_(gws.x_), y_(gws.y_) {}

    ~MOGridWorldState() {}

    virtual mlcore::State& operator=(const mlcore::State& rhs)
    {
        if (this == &rhs)
            return *this;

        MOGridWorldState* gws = (MOGridWorldState *)  & rhs;
        x_ =  gws->x_;
        y_=  gws->y_;
        return *this;
    }

    virtual bool operator==(const mlcore::State& rhs) const
    {
        MOGridWorldState* gws = (MOGridWorldState *)  &rhs;
        return x_ == gws->x_ && y_ == gws->y_;
    }

    virtual bool equals(mlcore::State* other) const;

    virtual int hashValue() const;

    int x() const;

    int y() const;
};

}
#endif // MDPLIB_MOGRIDWORLDSTATE_H
