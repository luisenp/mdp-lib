#ifndef MDPLIB_DUMMYSTATE_H
#define MDPLIB_DUMMYSTATE_H

#include <list>

#include "../State.h"


class DummyState : public mlcore::State
{
private:

    std::list<mlcore::Successor> successors_;

    virtual std::ostream& print(std::ostream& os) const;

public:

    DummyState() { }

    virtual ~DummyState() { }

    std::list<mlcore::Successor> successors() { return successors_; }

    void setSuccessors(std::list<mlcore::Successor> successors)
    {
        successors_ = successors;
    }

    /**
     * Overrides method from State.
     */
    virtual mlcore::State& operator=(const mlcore::State& rhs)
    {
        if (this == &rhs)
            return *this;

        DummyState* state = (DummyState*)  & rhs;
        successors_ = state->successors_;
        return *this;
    }

    /**
     * Overrides method from State.
     */
    virtual bool operator==(const mlcore::State& rhs) const
    {
        return this == &rhs;
    }

    /**
     * Overrides method from State.
     */
    virtual bool equals(mlcore::State* other) const
    {
        return this == other;
    }

    /**
     * Overrides method from State.
     */
    virtual int hashValue() const
    {
        return -1;
    }
};

#endif // MDPLIB_DUMMYSTATE_H
