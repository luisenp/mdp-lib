#ifndef MDPLIB_RACETRACKSTATE_H
#define MDPLIB_RACETRACKSTATE_H

#include <functional>

#include "../../state.h"
#include "../../problem.h"

typedef std::vector <std::list<mlcore::Successor> > all_successor_t;

class RacetrackState : public mlcore::State
{
private:
    int x_;
    int y_;
    int vx_;
    int vy_;

    /* A cache of all successors (for all actions) of this state */
    all_successor_t allSuccessors_;


    virtual std::ostream& print(std::ostream& os) const
    {
        os << "Racetrack State: (" << x_  << ", " << y_ << ", " << vx_ << ", " << vy_ << ")";
    }

public:
    /**
     * Creates a state for the racetrack problem with the given (x,y) position
     * and (vx,vy) velocity, and assigned to the given index.
     *
     * Every tuple (x, y, vx, vy) should be assigned to a unique index.
     */
    RacetrackState(int x, int y, int vx, int vy, mlcore::Problem* problem)
    {
        x_ = x;
        y_ = y;
        vx_ = vx;
        vy_ = vy;
        problem_ = problem;

        /* Adding a successor entry for each action */
        for (int i = 0; i < 9; i++) {
            allSuccessors_.push_back(std::list<mlcore::Successor> ());
        }
    }

    virtual ~RacetrackState() {}

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
    virtual mlcore::State& operator=(const mlcore::State& rhs)
    {
        if (this == &rhs)
            return *this;

        RacetrackState* state = (RacetrackState*)  & rhs;
        x_ = state->x_;
        y_ = state->y_;
        vx_ = state->vx_;
        vy_ = state->vy_;
        problem_ = state->problem_;
        return *this;
    }

    /**
     * Overrides method from State.
     */
    virtual bool operator==(const mlcore::State& rhs) const
    {
        RacetrackState* state = (RacetrackState*)  & rhs;
        return x_ == state->x_
                && y_ == state->y_
                && vx_ == state->vx_
                && vy_ == state->vy_;
    }

    /**
     * Overrides method from State.
     */
    virtual bool equals(mlcore::State* other) const
    {
        RacetrackState* state = (RacetrackState*) other;
        return *this ==  *state;
    }

    /**
     * Overrides method from State.
     */
    virtual int hashValue() const
    {
        return x_ + 31*(y_ + 31*(vx_ + 31*vy_));
    }
};

#endif // MDPLIB_RACETRACKSTATE_H
