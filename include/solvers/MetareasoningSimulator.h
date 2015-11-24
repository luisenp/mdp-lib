#ifndef METAREASONINGSIMULATOR_H
#define METAREASONINGSIMULATOR_H

#include "../problem.h"

namespace mlsolvers
{

/**
 * Class used to simulate an optimal metareasoning algorithm
 * based on Value Iteration.
 * It requires discounting (i.e., gamma < 1.0).
 */
class MetareasoningSimulator
{
private:

    /* The problem to solve. */
    mlcore::Problem* problem_;

    /* Residual tolerance used as a stopping condition for VI. */
    double tolerance_;

    /* Expected cost of the policies at each iteration of VI. */
    std::vector< mlcore::StateDoubleMap> policyCosts_;

    /* The estimated values at each iteration of VI. */
    std::vector< mlcore::StateDoubleMap> stateValues_;

    /* Computes the expected cost of VI's current policy. */
    void computeExpectedCostCurrentPolicy(
        mlcore::StateDoubleMap& expectedCosts_);

public:
    MetareasoningSimulator(mlcore::Problem* problem, double tolerance = 1.0e-6)
    {
        problem_ = problem;
        tolerance_ = tolerance;
    }

    virtual ~MetareasoningSimulator() { }

    /**
     * Precomputes and stores the expected costs
     * of all the intermediate policies found during Value Iteration.
     */
    void precomputeAllExpectedPolicyCosts();

    void simulate();
};

} // mlsolvers


#endif // METAREASONINGSIMULATOR_H
