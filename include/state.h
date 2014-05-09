#ifndef MDPLIB_STATE_H
#define MDPLIB_STATE_H

#include <iostream>
#include <string>
#include <unordered_set>

#include "util/rational.h"

/**
 * Abstract class for states.
 */
class State
{
private:
    bool visited_;

protected:
    virtual std::ostream& print(std::ostream& stream) const =0;

public:
    virtual State & operator=(const State& rhs) =0;

    virtual bool operator==(const State& rhs) const =0;

    friend std::ostream& operator<<(std::ostream& os, const State* s);

    /**
     * Returns a hash value for the state.
     */
    virtual int hash_value() const =0;

    /**
     * Returns true if the state has been visited by some search-based algorithm.
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
};

/**
 * A state with no particular information inside.
 */
class DummyState : public State
{
public:
    DummyState() {}

    virtual State & operator=(const State& rhs)
    {
        return *this;
    }

    virtual bool operator==(const State& rhs) const
    {
        return this == &rhs;
    }

    virtual int hash_value() const
    {
        return 0;
    }
};

/**
 * A successor is just a <state, Rational> pair.
 */
 typedef std::pair<const State *, Rational> Successor;


/**
 * Wrapper of the hash function for state objects (used to define StateSet below).
 */
struct StateHash {
  size_t operator()(const State *s) const {
    return s->hash_value();
  }
};

/**
 * Wrapper of the equality operator for state objects (used to define StateSet below).
 */
struct StateEqual {
  bool operator() (const State *s1, const State *s2) const {
    return s1 == s2;
  }
};

/**
 * A set of states.
 */
typedef std::unordered_set<const State *, StateHash, StateEqual> StateSet;


#endif // MDPLIB_STATE_H
