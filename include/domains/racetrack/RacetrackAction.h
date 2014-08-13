#ifndef MDPLIB_RACETRACKACTION_H
#define MDPLIB_RACETRACKACTION_H

#include <cassert>

#include "../../problem.h"
#include "../../state.h"
#include "../../action.h"


/**
 * A class implementing actions in the racetrack domain.
 * There are 9 possible actions of the form (ax, ay) where ax and ay
 * are either -1, 0 or 1.
 */
class RacetrackAction : public mlcore::Action
{
private:
    int ax_;
    int ay_;

    virtual std::ostream& print(std::ostream& os) const
    {
        os << "Racetrack Action: (" << ax_ << ", " << ay_ << ")";
        return os;
    }

public:
    RacetrackAction(int ax, int ay)
    {
        assert(ax >= -1 && ax <= 1 && ay >= -1 && ay <= 1);
        ax_ = ax;
        ay_ = ay;
    }

    virtual ~RacetrackAction();

    /**
     * Overriding method from Action.
     */
    virtual mlcore::Action& operator=(const mlcore::Action& rhs)
    {
        if (this == &rhs)
            return *this;

        RacetrackAction* action = (RacetrackAction*)  & rhs;
        ax_ =  action->ax_;
        ay_ =  action->ay_;
        return *this;
    }

    /**
     * Overriding method from Action.
     */
    virtual int hashValue() const
    {
        return (int) 3*(ax_ + 1) + ay_ + 1;
    }
};

#endif // MDPLIB_RACETRACKACTION_H
