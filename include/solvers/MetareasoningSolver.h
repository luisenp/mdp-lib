#ifndef MDPLIB_METAREASONINGSOLVER_H
#define MDPLIB_METAREASONINGSOLVER_H

#include "solver.h"

namespace mlsolvers
{

/**
 * A metareasoning solver based on LAO*.
 */
class MetareasoningSolver : public Solver
{
private:
    mlcore::Problem* problem_;
    mlcore::StateSet visited_;

    bool useMetareasoning_ = false;

    /* Estimates the Q-Value of a state-action pair given the current state of the planner. */
    double estimateQValueAction(mlcore::State* s, mlcore::Action* a);

    /* Expands the BPSG rooted at state s and returns the number of states expanded */
    int expand(mlcore::State* s);

public:
    MetareasoningSolver(mlcore::Problem* problem) : problem_(problem) { }
    virtual ~MetareasoningSolver() { }

    void useMetareasoning(bool value) { useMetareasoning_ = value; }

    /**
     * Returns an action for the given state using the metareasoning algorithm.
     *
     * @param s The state for which the action is requested.
     */
    virtual mlcore::Action* solve(mlcore::State* s);
};

} // mlsolvers

#endif // MDPLIB_METAREASONINGSOLVER_H
