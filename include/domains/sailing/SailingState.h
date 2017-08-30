#ifndef MDPLIB_SAILINGSTATE_H
#define MDPLIB_SAILINGSTATE_H

#include "../../State.h"

#include "SailingNoWindHeuristic.h"

class SailingNoWindHeuristic;

class SailingState : public mlcore::State
{

friend SailingNoWindHeuristic;

private:
    short x_;
    short y_;
    short wind_;

    virtual std::ostream& print(std::ostream& os) const
    {
        os << "Sailing State: " << x_ << " " << y_ << " " << wind_;
        return os;
    }

public:
    SailingState(short x, short y, short wind, mlcore::Problem* problem)
    {
        x_ = x;
        y_ = y;
        wind_ = wind;
        problem_ = problem;
    }

    virtual ~SailingState() {}

    short x() const { return x_; }

    short y() const { return y_; }

    short wind() const { return wind_; }

    /**
     * Overrides method from State.
     */
    virtual mlcore::State& operator=(const mlcore::State& rhs)
    {
        if (this == &rhs)
            return *this;

        const SailingState* state = static_cast<const SailingState*> (&rhs);
        x_ =  state->x_;
        y_ =  state->y_;
        wind_ = state->wind_;
        problem_ = state->problem_;
        return *this;
    }

    /**
     * Overrides method from State.
     */
    virtual bool operator==(const mlcore::State& rhs) const
    {
        const SailingState* state = static_cast<const SailingState*> (&rhs);
        return x_ == state->x_
                && y_ == state->y_
                && wind_ == state->wind_;
    }

    /**
     * Overrides method from State.
     */
    virtual bool equals(mlcore::State* other) const
    {
        SailingState* state = static_cast<SailingState*> (other);
        return *this ==  *state;
    }

    /**
     * Overrides method from State.
     */
    virtual int hashValue() const
    {
        return 31*(x_ + 31*(y_ + 31*wind_));
    }
};

#endif // MDPLIB_SAILINGSTATE_H
