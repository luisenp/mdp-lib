#ifndef MDPLIB_CTPSTATE_H
#define MDPLIB_CTPSTATE_H

#include <vector>

#include "../../state.h"

class CTPProblem;

/**
 * A class implementing states in the Canadian Traveler Problem.
 */
class CTPState : public State
{
private:
    int location_;
    std::vector< std::vector <unsigned char> > status_;

    virtual std::ostream& print(std::ostream& os) const;

    void initAllUnkown();

public:
    CTPState() {}

    /**
     * Creates a state associated to the given problem with the agent
     * at location 0 and the status of all roads set to "unknown".
     */
    CTPState(CTPProblem* problem);

    /**
     * Creates a state associated to the given problem with the agent
     * at the given location and the status of all roads set to "unknown".
     */
    CTPState(CTPProblem* problem, int location);

    /**
     * Creates a state that copies the given state.
     */
    CTPState(CTPState& rhs);

    virtual ~CTPState() {}

    int location()
    {
        return location_;
    }

    std::vector< std::vector <unsigned char> >& status()
    {
        return status_;
    }

    virtual State& operator=(const State& rhs)
    {
        if (this == &rhs)
            return *this;

        CTPState* state = (CTPState*)  & rhs;
        location_ =  state->location_;
        status_ =  state->status_;
        return *this;
    }

    virtual bool operator==(const State& rhs) const
    {
        CTPState* state = (CTPState*)  & rhs;
        return location_ == state->location_ && status_ == state->status_;
    }

    virtual bool equals(State* other) const;
    virtual int hashValue() const;
};

#endif // MDPLIB_CTPSTATE_H
