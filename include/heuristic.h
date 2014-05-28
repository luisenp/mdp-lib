#ifndef MDPLIB_HEURISTIC_H
#define MDPLIB_HEURISTIC_H

#include "state.h"
#include "util/rational.h"

class Heuristic
{
public:
    virtual Rational cost(const State* s) const =0;
};

#endif // MDPLIB_HEURISTIC_H
