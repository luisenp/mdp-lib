#ifndef MDPLIB_LEXIRACETRACKSTATE_H
#define MDPLIB_LEXIRACETRACKSTATE_H

#include <functional>

#include "../../state.h"
#include "../../problem.h"

#include "../../../include/domains/racetrack/RacetrackState.h"

namespace mllexi
{

class LexiRacetrackState : public LexiState
{
private:
    int x_;
    int y_;
    int vx_;
    int vy_;

    /* A cache of all successors (for all actions) of this state */
    all_successor_t allSuccessors_;

    virtual std::ostream& print(std::ostream& os) const;

public:
    /**
     * Creates a state for the racetrack problem with the given (x,y) position
     * and (vx,vy) velocity, and assigned to the given index.
     *
     * Every tuple (x, y, vx, vy) should be assigned to a unique index.
     */
    LexiRacetrackState(int x, int y, int vx, int vy, LexiProblem* problem);

    virtual ~LexiRacetrackState() {}

    int x() const { return x_; }

    int y() const { return y_; }

    int vx() const { return vx_; }

    int vy() const { return vy_; }

    /**
     * Returns a pointer to the successor cache of this state.
     */
    all_successor_t* allSuccessors() { return &allSuccessors_; }

    /**
     * Overrides method from State.
     */
    virtual mlcore::State& operator=(const mlcore::State& rhs);

    /**
     * Overrides method from State.
     */
    virtual bool operator==(const mlcore::State& rhs) const;

    /**
     * Overrides method from State.
     */
    virtual bool equals(mlcore::State* other) const;

    /**
     * Overrides method from State.
     */
    virtual int hashValue() const;
};

}

#endif // MDPLIB_LEXIRACETRACKSTATE_H
