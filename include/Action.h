#ifndef MDPLIB_ACTION_H
#define MDPLIB_ACTION_H

#include <iostream>
#include <unordered_set>
#include <unordered_map>

namespace mlcore
{

class Action
{
protected:
    virtual std::ostream& print(std::ostream& os) const =0;

public:
    virtual ~Action() {}

    /**
     * Returns a hash value for the action.
     *
     * @return The hash value of the action.
     */
    virtual int hashValue() const=0;

    virtual Action& operator=(const Action& rhs) =0;

    friend std::ostream& operator<<(std::ostream& os, Action* a);
};

/**
 * Wrapper of the hash function for action objects (used to define
 * ActionSet below).
 */
struct ActionHash {
  size_t operator()(Action* a) const {
    return a->hashValue();
  }
};

/**
 * Wrapper of the equality operator for action objects (used to define
 * ActionSet below).
 */
struct ActionEqual {
  bool operator() (Action* a1, Action* a2) const {
    /*
     * Assuming actions can be compared by their address since no new actions
     * are generated after the problem is created.
     */
    return a1 == a2;
  }
};

/**
 * A set of actions.
 */
typedef std::unordered_set<Action*, ActionHash, ActionEqual> ActionSet;

/**
 * A map of actions to integers.
 */
typedef std::unordered_map<Action*, int, ActionHash, ActionEqual> ActionIntMap;

/**
 * A map of actions to doubles.
 */
typedef
std::unordered_map<Action*, double, ActionHash, ActionEqual> ActionDoubleMap;

}

#endif // MDPLIB_ACTION_H
