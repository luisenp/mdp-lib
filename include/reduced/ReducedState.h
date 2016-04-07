#ifndef MDPLIB_REDUCEDSTATE_H
#define MDPLIB_REDUCEDSTATE_H

#include "../State.h"

namespace mlreduced
{

/**
 * A class that implements states for a reduced model.
 */
class ReducedState : public mlcore::State
{
protected:
    /**
     * The state in the original MDP.
     */
    mlcore::State* originalState_;

    /**
     * The number of exceptions that have occurred so far.
     */
    int transitionCount_;

public:
    ReducedState()
    {
        originalState_ = nullptr;
        problem_ = nullptr;
        transitionCount_ = 0;
    }

    ReducedState(const ReducedState& rhs) :
        originalState_(rhs.originalState_),
        transitionCount_(rhs.transitionCount_)
    {
        problem_ = rhs.problem_;
    }

    ReducedState(mlcore::State* originalState,
                 int transitionCount,
                 mlcore::Problem* problem) :
        originalState_(originalState),
        transitionCount_(transitionCount)
    {
        problem_ = problem;
    }

    virtual ~ReducedState() {}

    mlcore::State* originalState() { return originalState_; }

    void originalState(mlcore::State* value) { originalState_ = value; }

    int transitionCount() { return transitionCount_; }

    void transitionCount(int value) { transitionCount_ = value; }

    virtual mlcore::State& operator=(const mlcore::State& rhs)
    {
        if (this == &rhs)
            return *this;

        ReducedState* rs = (ReducedState *) &rhs;
        originalState_ = rs->originalState_;
        transitionCount_ = rs->transitionCount_;
        problem_ = rs->problem_;
        return *this;
    }

    virtual bool operator==(const mlcore::State& rhs) const
    {
        ReducedState* rs = (ReducedState *) &rhs;
        return this->originalState_ == rs->originalState_ &&
                this->transitionCount_ == rs->transitionCount_;
    }

    virtual bool equals(mlcore::State* other) const
    {
        ReducedState* rs = (ReducedState *) other;
        return *this == *other;
    }

    virtual int hashValue() const
    {
        return originalState_->hashValue() + 37 * transitionCount_;
    }


    virtual std::ostream& print(std::ostream& os) const
    {
        os << "RS:(" << originalState_ << ", " << transitionCount_ << ")";
        return os;
    }

};

}

#endif // MDPLIB_REDUCEDSTATE_H
