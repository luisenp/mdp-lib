#ifndef METAREASONINGSIMULATOR_H
#define METAREASONINGSIMULATOR_H

#include "../problem.h"

enum ActionSelectionRule
{
    META_ASSUMPTION_1,
    META_ASSUMPTION_2,
    NO_META
};

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

    /* The number of planning steps per action execution. */
    int numPlanningStepsPerAction_;

    /* The number of planning steps per NOP action. */
    int numPlanningStepsPerNOP_;

    /* The rule to use to select actions. */
    ActionSelectionRule rule_;

    /* The cost of executing a NOP. */
    double costNOP_;

    /*
     * Returns the action chosen using the following Q-values:
     *
     *   Q(s,NOP) = C(s, NOP) + V[t + dt][s]
     *   Q(s, a) = C(s,a) + sum_s' T(s',s,a) EC[t + dta][s']
     *
     *  where:
     *     dtnop := numPlanningStepsPerNOP_
     *     dta := numPlanningStepsPerAction_
     *     EC := policyCosts_
     *     C(s, NOP) = costNOP_
     */
    mlcore::Action* getActionMetaAssumption1(mlcore::State* s, int t);

public:
    MetareasoningSimulator(mlcore::Problem* problem,
                        double tolerance = 1.0e-6,
                        int numPlanninStepsPerAction = 5,
                        int numPlanningStepsPerNOP = 5,
                        double costNOP = 1.0,
                        ActionSelectionRule rule = META_ASSUMPTION_1)
    {
        problem_ = problem;
        tolerance_ = tolerance;
        numPlanningStepsPerAction_ = numPlanninStepsPerAction;
        numPlanningStepsPerNOP_ = numPlanningStepsPerNOP;
        costNOP_ = costNOP;
        rule_ = rule;
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
