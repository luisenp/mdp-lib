#ifndef MDPLIB_RACETRACKSTATE_H
#define MDPLIB_RACETRACKSTATE_H

#include <functional>

#include "../../Problem.h"
#include "../../State.h"


class RacetrackState : public mlcore::State
{
private:
    int x_;
    int y_;
    int vx_;
    int vy_;

    /* A cache of all successors (for all actions) of this state */
    std::vector<mlcore::SuccessorsList> allSuccessors_;

    virtual std::ostream& print(std::ostream& os) const;

public:
    /**
     * Creates a state for the racetrack problem with the given (x,y) position
     * and (vx,vy) velocity, and assigned to the given index.
     *
     * Every tuple (x, y, vx, vy) should be assigned to a unique index.
     */
    RacetrackState(int x, int y, int vx, int vy, mlcore::Problem* problem);

    virtual ~RacetrackState() {}

    int x() const { return x_; }

    int y() const { return y_; }

    int vx() const { return vx_; }

    int vy() const { return vy_; }

    /**
     * Returns a pointer to the successor cache of this state.
     */
    std::vector<mlcore::SuccessorsList>* allSuccessors()
    {
        return &allSuccessors_;
    }

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

#endif // MDPLIB_RACETRACKSTATE_H
