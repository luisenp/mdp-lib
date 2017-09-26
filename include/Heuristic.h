#ifndef MDPLIB_HEURISTIC_H
#define MDPLIB_HEURISTIC_H

#include "State.h"

namespace mlcore
{

/**
 * An interface for heuristic functions.
 */
class Heuristic
{
public:
    virtual ~Heuristic() {}

    /**
     * Returns an estimate of reaching a goal state from the given state.
     */
    virtual double cost(const State* s)=0;
};

}

#endif // MDPLIB_HEURISTIC_H
