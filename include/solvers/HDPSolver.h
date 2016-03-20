#ifndef MDPLIB_HDPSOLVER_H
#define MDPLIB_HDPSOLVER_H

#include <list>

#include "../Problem.h"
#include "../State.h"

#include "Solver.h"

namespace mlsolvers
{

/**
 * An implementation of the HDP and HPD(i,j) algorithms as described
 * in http://web.cs.ucla.edu/pub/stat_ser/R317.pdf (IJCAI'03).
 */
class HDPSolver : public Solver
{
private:
    /* The problem to solve. */
    mlcore::Problem* problem_;

    /* The tolerance for the residual error. */
    double epsilon_;

    /* Performs a DFS while simultaneously labeling states as solved. */
    bool dfs(mlcore::State* s);

    /* An index used to compute the connected component algorithm. */
    int index_;

    /* A stack of states used to compute the connected components. */
    std::list<mlcore::State*> stateStack_;

    /* A map from state to indices used to compute the connected components. */
    mlcore::StateIntMap indices_;

    /* A map from state to indices used to compute the connected components. */
    mlcore::StateIntMap low_;

    /* The set of states in the connected components stack. */
    mlcore::StateSet inStack_;

public:
    HDPSolver(mlcore::Problem* problem, double epsilon) :
        problem_(problem), epsilon_(epsilon) { }

    virtual ~HDPSolver() { }

    /**
     * Solves the associated problem using the HDP algorithm.
     *
     * @param s0 The state to start the search at.
     */
    virtual mlcore::Action* solve(mlcore::State* s0);
};

} // namespace mlsolvers

#endif // MDPLIB_HDPSOLVER_H
