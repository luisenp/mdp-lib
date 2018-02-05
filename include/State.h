#ifndef MDPLIB_STATE_H
#define MDPLIB_STATE_H

#include <iostream>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "Action.h"
#include "MDPLib.h"

#define su_state first
#define su_prob second

namespace mlcore
{

class Problem;

/**
 * Abstract class for states.
 *
 * For the planning algorithms to work, it's important that all states have
 * the variable "problem_" set to the problem they correspond to.
 */
class State
{
protected:

    /**
     * A bit mask that is helpful to speed up solvers.
     */
    unsigned long bits_;

    /**
     * An estimate of the expected cost of reaching the goal from this state.
     */
    double cost_;

    /**
     * For weighted methods such as weighted-LAO*.
     */
    double gValue_;

    /**
     * For weighted methods such as weighted-LAO*.
     */
    double hValue_;

    /**
     * An estimate of the best action to reach a goal from this state.
     */
    Action* bestAction_;

    /**
     * An estimate of the number of steps needed to find a state with high
     * residual error.
     */
    double residualDistance_;

    /**
     * The current depth of the state during search (useful for some methods).
     */
    double depth_;

    /**
     * The problem to which this state belongs.
     */
    Problem* problem_;

    /**
    * Whether this state was found to be a dead-end or not.
    */
    bool deadEnd_;

    virtual std::ostream& print(std::ostream& os) const =0;

public:

    /**
     * Common constructor. Used to initialize inherited variables.
     */
    State() : bits_(0),
              cost_(mdplib::dead_end_cost + 1),
              gValue_(mdplib::dead_end_cost + 1),
              hValue_(mdplib::dead_end_cost + 1),
              bestAction_(nullptr),
              problem_(nullptr),
              deadEnd_(false),
              residualDistance_(mdplib::no_distance),
              depth_(mdplib::no_distance)
    { }

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
    bool checkBits(unsigned long bitMask) const
    {
        return bits_ & bitMask;
    }

    /**
     * Marks this state as a dead-end.
     */
    void markDeadEnd()
    {
        deadEnd_ = true;
    }

    /**
     * Returns whether this state is a dead-end or not.
     *
     * @return whether this state is a dead-end or not.
     */
    bool deadEnd() const
    {
        return deadEnd_;
    }

    /**
     * Returns an estimate of the optimal expected cost to reach
     * a goal from this state.
     *
     * @return An estimate of the optimal expected cost to reach a goal from
     * this state.
     */
    double cost() const;

    /**
     * Updates the estimate of the expected cost to reach a goal from this
     * state.
     *
     * @param c The updated expected cost to reach a goal from this state.
     */
    void setCost(double c)
    {
        cost_ = c;
    }

    /**
     * Returns the g-value of the state (for weighted methods, e.g. w-LAO*)
     *
     * @return The g-value of the state.
     */
    double gValue() const;

    /**
     * Updates the g-value of the state (for weighted methods, e.g. w-LAO*).
     *
     * @param g The g-value of the state.
     */
    void gValue(double g)
    {
        gValue_ = g;
    }

    /**
     * Returns the h-value of the state (for weighted methods, e.g. w-LAO*)
     *
     * @return The h-value of the state.
     */
    double hValue() const;

    /**
     * Updates the h-value of the state (for weighted methods, e.g. w-LAO*).
     *
     * @param h The h-value of the state.
     */
    void hValue(double h)
    {
        hValue_ = h;
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
     * Updates the estimate of the expected cost to reach a goal from this
     * state.
     *
     * @param a The updated best action estimate for the state.
     */
    void setBestAction(Action* a)
    {
        bestAction_ = a;
    }


    /**
     * Estimated distance to high residual errors.
     *
     * @return The estimated distance.
     */
    double residualDistance() const
    {
        return residualDistance_;
    }

    /**
     * Updates the estimate to the distance to high residual errors.
     *
     * @param value The value to use for the update.
     */
    void residualDistance(double value)
    {
        residualDistance_ = value;
    }


    /**
     * The depth of the state on a search tree.
     *
     * @return The depth.
     */
    double depth() const
    {
        return depth_;
    }

    /**
     * Updates the depth of the state.
     *
     * @param value The value to use for the depth.
     */
    void depth(double value)
    {
        depth_ = value;
    }

    /**
     * Resets all instance variables used by solvers to their default values.
     * The following variables are reset: visited, bestAction and cost.
     */
    void reset()
    {
        bits_ = 0;
        cost_ = mdplib::dead_end_cost + 1;
        bestAction_ = nullptr;
        depth_ = mdplib::no_distance;
        residualDistance_ = mdplib::no_distance;
    }
};

/**
 * A successor is just a <State*, double> pair.
 */
 typedef std::pair<State*, double> Successor;

/**
 * Wrapper of the hash function for state objects
 * (used to define StateSet below).
 */
struct StateHash {
  size_t operator()(State* s) const {
    return s->hashValue();
  }
};

/**
 * Wrapper of the equality operator for state objects
 * (used to define StateSet below).
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

/**
 * A map of states to integers.
 */
typedef std::unordered_map<State*, int, StateHash, StateEqual> StateIntMap;

/**
 * A map of states to doubles.
 */
typedef
std::unordered_map<State*, double, StateHash, StateEqual> StateDoubleMap;

/**
 * A map of states to actions.
 */
typedef
std::unordered_map<State*, Action*, StateHash, StateEqual> StateActionMap;

/**
 * A list of successors.
 * TODO change every instance of std::list<mlcore::Successor> to SuccessorsList.
 */
typedef std::list<Successor> SuccessorsList;

}   // namespace mlcore

#endif // MDPLIB_STATE_H
