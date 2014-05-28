#ifndef MDPLIB_STATE_H
#define MDPLIB_STATE_H

#include <iostream>
#include <string>
#include <unordered_set>

#include "action.h"
#include "util/rational.h"

#define su_state first
#define su_prob second

/**
 * Abstract class for states.
 */
class State
{
private:
    bool visited_ = false;
    Rational cost_ = Rational(0);
    Action *bestAction_ = 0;

protected:
    virtual std::ostream& print(std::ostream& os) const =0;

public:
    virtual State & operator=(const State& rhs) =0;

    virtual bool operator==(const State& rhs) const =0;

    friend std::ostream& operator<<(std::ostream& os, State* s);

    /**
     * Equality function used for unordered sets/maps.
     *
     * @return true if this state equals the given state.
     */
    virtual bool equals(State *other) const =0;

    /**
     * State hash function used for unordered sets/maps.
     *
     * @return A hash value of the state.
     */
    virtual int hashValue() const =0;

    /**
     * Returns true if the state has been visited by some search-based algorithm.
     *
     * @return true if the state has been visited by some search-based algorithm.
     */
    bool visited() const
    {
        return visited_;
    }

    /**
     * Marks the state as visited by some search-based algorithm.
     */
    void visit()
    {
        visited_ = true;
    }

    /**
     * Marks the state as not-visited by some search-based algorithm.
     */
    void unvisit()
    {
        visited_ = false;
    }

    /**
     * Returns an estimate of the optimal expected cost to reach
     * a goal from this state.
     *
     * @return An estimate of the optimal expected cost to reach a goal from this state.
     */
    Rational cost() const
    {
        return cost_;
    }

    /**
     * Updates the estimate of the expected cost to reach a goal from this state.
     *
     * @param c The updated expected cost to reach a goal from this state.
     */
    void setCost(Rational c)
    {
        cost_ = c;
    }

    /**
     * Estimated best action for the state.
     *
     * @return The current best action for this state.
     */
    Action* bestAction() const
    {
        return bestAction_;
    }

    /**
     * Updates the estimate of the expected cost to reach a goal from this state.
     *
     * @param a The updated best action estimate for the state.
     */
    void setBestAction(Action* a)
    {
        bestAction_ = a;
    }
};

/**
 * A successor is just a <State*, Rational> pair.
 */
 typedef std::pair<State*, Rational> Successor;


/**
 * Wrapper of the hash function for state objects (used to define StateSet below).
 */
struct StateHash {
  size_t operator()(State* s) const {
    return s->hashValue();
  }
};

/**
 * Wrapper of the equality operator for state objects (used to define StateSet below).
 */
struct StateEqual {
  bool operator() (State* s1, State* s2) const {
    return s1->equals(s2);
  }
};

/**
 * A set of states.
 */
typedef std::unordered_set<State*, StateHash, StateEqual> StateSet;


#endif // MDPLIB_STATE_H
