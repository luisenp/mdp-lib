#ifndef MDPLIB_PPDDLSTATE_H
#define MDPLIB_PPDDLSTATE_H

#include "../state.h"
#include "mini-gpt/states.h"

namespace mlppddl
{

class State : public mlcore::State
{
private:
    state_t* pState_;

    virtual std::ostream& print(std::ostream& os) const;

public:

    State(mlcore::Problem* problem)
    {
        mlcore::State::problem_ = problem;
        pState_ = new state_t;
    }

    State(mlcore::Problem* problem, state_t* pState) : pState_(pState)
    {
        mlcore::State::problem_ = problem;
    }

    virtual ~State()
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

        State* state = (State*)  & rhs;
        pState_ = state->pState_;
        problem_ = state->problem_;
        return *this;
    }

    /**
     * Overrides method from State.
     */
    virtual bool operator==(const mlcore::State& rhs) const
    {
        State* state = (State*)  & rhs;
        return *pState_ == *state->pState_;
    }

    /**
     * Overrides method from State.
     */
    virtual bool equals(mlcore::State* rhs) const
    {
        State* state = (State*)  & rhs;
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
