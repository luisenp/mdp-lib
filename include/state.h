#ifndef MDPLIB_STATE_H
#define MDPLIB_STATE_H

#include <string>
#include <unordered_set>

#include "../include/util/rational.h"

/**
 * Abstract class for states.
 */
class State
{
private:
    bool visited_;

public:
    State();

    State(const State *other);

    virtual ~State();

    virtual State *operator=(const State *other) =0;

    virtual bool operator==(const State *other) const =0;

    virtual std::string to_string() const =0;

    virtual int hash_value() const =0;

    bool visited() const
    {
        return visited_;
    }

    void visit()
    {
        visited_ = true;
    }

    void unvisit()
    {
        visited_ = false;
    }
};

/**
 * A successor is just a <state, Rational> pair.
 */
 typedef std::pair<State *, Rational> Successor;


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
