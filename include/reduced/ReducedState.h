#ifndef MDPLIB_REDUCEDSTATE_H
#define MDPLIB_REDUCEDSTATE_H

#include "../State.h"

namespace mlreduced
{

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
    int exceptionCount_;

public:
    ReducedState(mlcore::State* originalState, int exceptionCount) :
        originalState_(originalState), exceptionCount_(exceptionCount) { }

    virtual ~ReducedState() {}

    mlcore::State* originalState() { return originalState_; }

    int exceptionCount() { return exceptionCount_; }

    virtual mlcore::State& operator=(const mlcore::State& rhs)
    {
        if (this == &rhs)
            return *this;

        ReducedState* rs = (ReducedState *) &rhs;
        originalState_ = rs->originalState_;
        exceptionCount_ = rs->exceptionCount_;
        return *this;
    }

    virtual bool operator==(const mlcore::State& rhs) const
    {
        ReducedState* rs = (ReducedState *) &rhs;
        return this->originalState_ == rs->originalState_ &&
                this->exceptionCount_ == rs->exceptionCount_;
    }

    virtual bool equals(mlcore::State* other) const
    {
        ReducedState* rs = (ReducedState *) other;
        return *this == *other;
    }

    virtual int hashValue() const
    {
        return originalState_->hashValue() + 37 * exceptionCount_;
    }


    virtual std::ostream& print(std::ostream& os) const
    {
        os << "RS:(" << originalState_ << "," << exceptionCount_ << ")";
        return os;
    }

};

}

#endif // MDPLIB_REDUCEDSTATE_H
