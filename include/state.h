#ifndef MDPLIB_STATE_H
#define MDPLIB_STATE_H

#include <iostream>
#include <string>
#include <unordered_set>

#include "action.h"
#include "mdplib.h"
#include "util/rational.h"

#define su_state first
#define su_prob second

class Problem;

/**
 * Abstract class for states.
 */
class State
{
protected:
    /**
     * A bit mask that is helpful to speed up solvers.
     */
    unsigned long bits_ = 0;

    /**
     * An estimate of the expected cost of reaching the goal from this state.
     */
    Rational cost_ = Rational(mdplib::dead_end_cost + 1);

    /**
     * An estimate of the best action to reach a goal from this state.
     */
    Action* bestAction_ = nullptr;

    /**
     * The problem to which this state belongs.
     */
    Problem* problem_ = nullptr;

    virtual std::ostream& print(std::ostream& os) const =0;

public:
    virtual ~State() {}

    virtual State& operator=(const State& rhs) =0;

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
     * Returns the bit mask associated to this state.
     *
     * @return The bit mask associated to this state.
     */
    long bits() const
    {
        return bits_;
    }

    /**
     * Activates the bits that are activated in the given bit mask.
     *
     * @param bitMask A mask that specifies the bits to be activated.
     */
    void setBits(unsigned long bitMask)
    {
        bits_ |= bitMask;
    }

    /**
     * Clears the bits that are activated in the given bit mask.
     *
     * @param bitMask A mask that specifies the bits to be cleared.
     */
    void clearBits(unsigned long bitMask)
    {
        bits_ &= ~bitMask;
    }

    /**
     * Checks if the (single) bit specified by the given bit mask is activated.
     * Doesn't work for bit masks with multiple bits activated.
     *
     * @param bitMask A mask that specifies the bits to be checked.
     */
    bool checkBits(unsigned long bitMask)
    {
        return bits_ & bitMask;
    }

    /**
     * Returns an estimate of the optimal expected cost to reach
     * a goal from this state.
     *
     * @return An estimate of the optimal expected cost to reach a goal from this state.
     */
    Rational cost() const;

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

    /**
     * Resets all instance variables used by solvers to their default values.
     * The following variables are reset: visited, bestAction and cost.
     */
    void reset()
    {
        bits_ = 0;
        cost_ = Rational(mdplib::dead_end_cost + 1);
        bestAction_ = nullptr;
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
