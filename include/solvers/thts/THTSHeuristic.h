#ifndef MDPLIB_THTSHEURISTIC_H
#define MDPLIB_THTSHEURISTIC_H

#include "../../Action.h"
#include "../../Problem.h"
#include "../../State.h"

namespace mlsolvers
{

// A heuristic for THTS solvers.
class THTSHeuristic {
public:
    // Returns a heuristic estimate for the value of the state and action pair.
    virtual double value(mlcore::State* state, mlcore::Action* action) =0;

};

} // namespace mlsolvers


#endif // MDPLIB_THTSHEURISTIC_H
