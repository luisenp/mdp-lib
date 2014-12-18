#ifndef MDPLIB_LEXIGRIDWORLDSTATE_H
#define MDPLIB_LEXIGRIDWORLDSTATE_H

#include "../lexi_state.h"

namespace mllexi
{

/**
 * A class representing a lexicographical version of the grid world state as described
 * in AIAMA 3rd Edition.
 */
class LexiGridWorldState : public LexiState
{
private:
    int x_;
    int y_;

    virtual std::ostream& print(std::ostream& os) const;

public:

    /**
     * Constructs a LexiGridWorldState representing grid position (x,y) on the problem
     * given as a first parameter.
     */
    LexiGridWorldState(mlcore::Problem* problem, int x, int y);

    /**
     * Copy constructor. The resulting state represents the same position as the
     * state passed as parameter.
     */
    LexiGridWorldState(const LexiGridWorldState& gws) : x_(gws.x_), y_(gws.y_) {}

    ~LexiGridWorldState() {}

    virtual mlcore::State& operator=(const mlcore::State& rhs)
    {
        if (this == &rhs)
            return *this;

        LexiGridWorldState* gws = (LexiGridWorldState *)  & rhs;
        x_ =  gws->x_;
        y_=  gws->y_;
        return *this;
    }

    virtual bool operator==(const mlcore::State& rhs) const
    {
        LexiGridWorldState* gws = (LexiGridWorldState *)  &rhs;
        return x_ == gws->x_ && y_ == gws->y_;
    }

    virtual bool equals(mlcore::State* other) const;

    virtual int hashValue() const;

    int x() const;

    int y() const;
};

}
#endif // MDPLIB_LEXIGRIDWORLDSTATE_H
