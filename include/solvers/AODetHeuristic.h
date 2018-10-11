#ifndef MDPLIB_AODETHEURISTIC_H
#define MDPLIB_AODETHEURISTIC_H


#include "../Heuristic.h"
#include "../Problem.h"

#include "../domains/AODeterminization.h"


namespace mlsolvers
{

/**
 * Implements the optimal solution to the all-outcomes determinization.
 */
class AODetHeuristic : public mlcore::Heuristic {
private:
    /* The problem for which this heuristic is computed. */
    mlcore::Problem* problem_;

    /* The all-outcomes determinization of the original problem. */
    AllOutcomesDeterminization* aodet_;

    /* Stores the computed costs. */
    mlcore::StateDoubleMap costs_;

    /* Solves the all-outcomes determinization using Value Iteration. */
    void solveVI(int maxIter = 100000, double tol = 1.0e-3);

    /*
     * Performs a Bellman update for the all-outcomes determinization.
     * We use this method instead of [Solver::bellmanUpdate] for efficiency
     * reasons.
     */
    double AODetBellmanUpdate(mlcore::State* s);

public:
    AODetHeuristic(mlcore::Problem* problem);

    virtual ~AODetHeuristic() { }

    virtual double cost(const mlcore::State* s);
};

}
#endif // MDPLIB_AODETHEURISTIC_H
