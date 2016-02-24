#ifndef MDPLIB_EPICSOLVER_H
#define MDPLIB_EPICSOLVER_H

#include "../Problem.h"
#include "../State.h"

#include "Solver.h"

namespace mlsolvers
{

class EpicSolver : public Solver
{
private:
    mlcore::Problem* problem_;

public:
    EpicSolver(mlcore::Problem* problem) : problem_(problem) { }

    virtual ~EpicSolver() { }

    virtual mlcore::Action* solve(mlcore::State* s0);

    /**
     * Computes the probability of reaching the given set of terminal states,
     * starting at the given initial state, using the greedy policy for the
     * states in the problem.
     *
     * Goals and dead-ends will be also be included in the set of terminals,
     * even if not present in the given set.
     *
     * @param start The initial state.
     * @param terminals The set of terminal states whose probability of reaching
     *                  will be computed.
     * @return A map from state to doubles storing the computed probabilities.
     */
    mlcore::StateDoubleMap computeProbabilityTerminals(
        mlcore::State* start, mlcore::StateSet& terminals);

};

}

#endif // MDPLIB_EPICSOLVER_H
