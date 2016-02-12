#ifndef MDPLIB_PPDDLSTATE_H
#define MDPLIB_PPDDLSTATE_H

#include "mini-gpt/states.h"

#include "../State.h"

namespace mlppddl
{

class PPDDLState : public mlcore::State
{
private:
    state_t* pState_;

    virtual std::ostream& print(std::ostream& os) const;

public:

    PPDDLState(mlcore::Problem* problem)
    {
        mlcore::State::problem_ = problem;
        pState_ = new state_t;
    }

    PPDDLState(mlcore::Problem* problem, state_t* pState) : pState_(pState)
    {
        mlcore::State::problem_ = problem;
    }

    virtual ~PPDDLState()
    {
        delete pState_;
    }

    state_t* pState() { return pState_; }

    void setPState(state_t & pState) { *pState_ = pState; }

    /**
     * Overrides method from State.
     */
    virtual mlcore::State& operator=(const mlcore::State& rhs)
    {
        if (this == &rhs)
            return *this;

        PPDDLState* state = (PPDDLState*)  & rhs;
        pState_ = state->pState_;
        problem_ = state->problem_;
        return *this;
    }

    /**
     * Overrides method from State.
     */
    virtual bool operator==(const mlcore::State& rhs) const
    {
        PPDDLState* state = (PPDDLState*)  & rhs;
        return *pState_ == *state->pState_;
    }

    /**
     * Overrides method from State.
     */
    virtual bool equals(mlcore::State* rhs) const
    {
        PPDDLState* state = (PPDDLState*)  & rhs;
        return *this == *rhs;
    }

    /**
     * Overrides method from State.
     */
    virtual int hashValue() const
    {
        return pState_->hash_value();
    }

};
}

#endif // MDPLIB_PPDDLSTATE_H
