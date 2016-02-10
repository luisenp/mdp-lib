#ifndef MDPLIB_CTPSTATE_H
#define MDPLIB_CTPSTATE_H

#include <vector>
#include <cassert>
#include <unordered_set>

#include "../../State.h"

class CTPProblem;

namespace ctp
{
    const int UNKNOWN = 15;
    const int TRUE = 1;
    const int FALSE = 2;
}

/**
 * A class implementing states in the Canadian Traveler Problem.
 */
class CTPState : public mlcore::State
{
private:
    int location_;
    std::vector< std::vector <unsigned char> > status_;
    std::unordered_set<int> explored_;
    unsigned char badWeather_ = ctp::UNKNOWN;

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

    virtual ~CTPState()
    {
        for (std::vector<unsigned char> v : status_)
            v.clear();
        status_.clear();
    }

    /**
     * Returns the location of the agent in this state.
     */
    int location()
    {
        return location_;
    }

    /**
     * Sets the location of the state to the given value.
     */
    void setLocation(int location)
    {
        location_ = location;
    }

    /**
     * Returns a set containing all vertices that have been explored so far
     * in this state.
     */
    std::unordered_set<int>& explored()
    {
        return explored_;
    }

    /**
     * Returns a 2D vector containing the status of the roads for this state.
     * Three status values are allowed: open, blocked and unknown. These are
     * defined in namespace ctp.
     */
    std::vector< std::vector <unsigned char> >& status()
    {
        return status_;
    }

    /**
     * Sets the status of the road between vertices i and j to the given value.
     */
    void setStatus(int i, int j, unsigned char st);

    /**
     * Checks if vertex v can be reached from vertex u given the known open roads.
     */
    bool reachable(int u);

    /**
     * Checks if vertex v can be reached from vertex u given the known blocked roads.
     */
    bool potentiallyReachable(int u);

    /**
     * Checks if this state is known to have bad weather.
     */
    bool badWeather();

    /**
     * Returns the cost of reaching location v from the agent's location in this state
     * using only open roads. It uses Dijkstra's shortest path algorithm.
     */
    double distanceOpen(int v);

    /**
     * Returns the cost of reaching location v from the agent's location in this state
     * using only open or unknown roads.
     * It uses Dijkstra's shortest path algorithm.
     */
    double distanceOptimistic(int v);

    /**
     * Overrides method from State.
     */
    virtual mlcore::State& operator=(const mlcore::State& rhs)
    {
        if (this == &rhs)
            return *this;

        CTPState* state = (CTPState*)  & rhs;
        location_ =  state->location_;
        status_ =  state->status_;
        explored_ = state->explored_;
        return *this;
    }

    /**
     * Overrides method from State.
     */
    virtual bool operator==(const mlcore::State& rhs) const
    {
        CTPState* state = (CTPState*)  & rhs;
        if (state->location_ == -1 && location_ == -1)
            return true;
        return location_ == state->location_
                && status_ == state->status_
                && explored_ == state->explored_;
    }

    /**
     * Overrides method from State.
     */
    virtual bool equals(mlcore::State* other) const;

    /**
     * Overrides method from State.
     */
    virtual int hashValue() const;
};

#endif // MDPLIB_CTPSTATE_H
