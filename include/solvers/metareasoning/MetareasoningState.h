#ifndef METAREASONINGSTATE_H
#define METAREASONINGSTATE_H

#include "../../state.h"

class MetareasoningState : public mlcore::State
{
private:
    mlcore::State* baseState_;

    int iteration_;

    virtual std::ostream& print(std::ostream& os) const;

public:
    MetareasoningState(mlcore::State* baseState, int iteration);

    virtual ~MetareasoningState() { }

    mlcore::State* baseState() { return baseState_; }

    int iteration() { return iteration_; }

    virtual mlcore::State& operator=(const mlcore::State& rhs)
    {
        if (this == &rhs)
            return *this;

        MetareasoningState* ms = (MetareasoningState*)  & rhs;
        baseState_ =  ms->baseState_;
        iteration_=  ms->iteration_;
        return *this;
    }

    virtual bool operator==(const mlcore::State& rhs) const
    {
        MetareasoningState* ms = (MetareasoningState *)  &rhs;
        return baseState_ == ms->baseState_ && iteration_ == ms->iteration_;
    }

    virtual bool equals(mlcore::State* other) const;
    virtual int hashValue() const;

};

#endif // METAREASONINGSTATE_H
