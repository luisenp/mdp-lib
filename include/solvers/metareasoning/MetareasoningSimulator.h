#ifndef METAREASONINGSIMULATOR_H
#define METAREASONINGSIMULATOR_H

#include "../../problem.h"

enum ActionSelectionRule
{
    META_ASSUMPTION_1,
    META_ASSUMPTION_2,
    META_CHANGE_ACTION,
    NO_META,
    OPTIMAL
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

    /* A metareasoning problem (only used if the action selection rule is OPTIMAL). */
    mlcore::Problem* metaProblem_;

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

    /* Whether to evaluate the current best action against NOP,
    * or all of the actions */
    bool tryAllActions_;

    /* The rule to use to select actions. */
    ActionSelectionRule rule_;

    /* The cost of executing a NOP. */
    double costNOP_;

    /*
     * Returns the action chosen using the following Q-values:
     *
     *  Q(s, NOP) = C(s, NOP) + EC[t + dtnop][s]
     *  Q(s, a) = C(s,a) + sum_s' T(s',s,a) EC[t + dta][s']
     *
     *  where:
     *     dtnop := numPlanningStepsPerNOP_
     *     dta := numPlanningStepsPerAction_
     *     EC := policyCosts_
     *     C(s, NOP) = costNOP_
     *     a := action chosen by the current plan
     *            (computed as in getActionNoMetareasoning)
     *
     * If Q(s, NOP) < Q(s, a), NOP is returned, otherwise a is returned.
     *
     * If the "tryAllActions_" variable is set to TRUE, then instead of using
     * the action chosen by the current plan, the method will look for the
     * action with the lowest Q(s,a) as computed above.
     */
    mlcore::Action* getActionMetaAssumption1(mlcore::State* s, int t);

    /*
     * Returns the action chosen using the following Q-values:
     *
     *  Q(s, NOP) = C(s, NOP) + V*[s]
     *  Q(s, a) = C(s,a) + sum_s' T(s',s,a) V*[s']
     *
     *  where:
     *     dtnop := numPlanningStepsPerNOP_
     *     dta := numPlanningStepsPerAction_
     *     V[s] := optimal value for the state s
     *     C(s, NOP) = costNOP_
     *     a := action chosen by the current plan
     *            (computed as in getActionNoMetareasoning)
     *
     * If Q(s, NOP) < Q(s, a), NOP is returned, otherwise a is returned.
     */
    mlcore::Action* getActionMetaAssumption2(mlcore::State* s, int t);

    /*
     * Returns the action chosen using the Values estimated by VI at each time
     * step:
     *  action = arg min_{a \in A} C(s,a) + sum_s' T(s',s,a) EC[t][s']
     */
    mlcore::Action* getActionNoMetareasoning(mlcore::State* s, int t);

    /*
     * Returns the action chosen according to the following rule:
     *     If the action chosen by the planner would change after one
     *     application of NOP, then use NOP. Otherwise, stick to the current
     *     action.
     */
    mlcore::Action* getActionMetaChangeBestAction(mlcore::State* s, int t);

    /*
     * Pre-computes and stores the expected costs
     * of all the intermediate policies found during Value Iteration.
     */
    void precomputeAllExpectedPolicyCosts();

public:
    MetareasoningSimulator(mlcore::Problem* problem,
                            double tolerance = 1.0e-6,
                            int numPlanningStepsPerAction = 5,
                            int numPlanningStepsPerNOP = 5,
                            double costNOP = 1.0,
                            bool tryAllActions = false,
                            ActionSelectionRule rule = META_ASSUMPTION_1);

    virtual ~MetareasoningSimulator()
    {
        if (metaProblem_ != nullptr)
            delete metaProblem_;
    }

    mlcore::Problem* problem() { return problem_; }

    double tolerance() { return tolerance_; }

    std::vector< mlcore::StateDoubleMap>& stateValues() { return stateValues_; };

    void rule(ActionSelectionRule value);

    void tryAllActions(bool value) { tryAllActions_ = value; }

    int numPlanningStepsPerAction() { return numPlanningStepsPerAction_; }

    void numPlanningStepsPerAction(int value) { numPlanningStepsPerAction_ = value; }

    int numPlanningStepsPerNOP() { return numPlanningStepsPerNOP_; }

    void numPlanningStepsPerNOP(int value) { numPlanningStepsPerNOP_ = value; }

    double costNOP() { return costNOP_; }

    /**
     * Simulates a run of a metareasoning approach that uses the information
     * collected by precomputeAllExpectedPolicyCosts.
     *
     * The method returns a pair with the cost accumulated during the
     * simulation and the total cost spent doing NOPs.
     */
    std::pair<double, double> simulate();
};

} // mlsolvers


#endif // METAREASONINGSIMULATOR_H
