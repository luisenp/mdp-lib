#ifndef MDPLIB_HEURISTIC_H
#define MDPLIB_HEURISTIC_H

#include "state.h"

namespace mlcore
{
    class Heuristic
    {
    public:
        virtual double cost(const State* s) const =0;
    };
}

#endif // MDPLIB_HEURISTIC_H
