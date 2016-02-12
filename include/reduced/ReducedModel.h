#ifndef MDPLIB_REDUCEDMODEL_H
#define MDPLIB_REDUCEDMODEL_H

#include <list>

#include "../solvers/Solver.h"

#include "../Action.h"
#include "../Problem.h"
#include "../State.h"

#include "ReducedState.h"
#include "ReducedTransitionConfig.h"

namespace mlreduced
{

class ReducedModel : public mlcore::Problem
{
protected:
    /**
     * The problem for which the reduced model is constructed.
     */
    mlcore::Problem *originalProblem_;

    /**
     * An object describing the outcomes to be considered primary for
     * each state, action pair.
     */
    ReducedTransitionConfig *config_;

    /**
     * The bound on the number of exceptions.
     */
    int k_;

    /**
     * If true, the reduced model will use the full transition function.
     */
    bool useFullTransition_;


    /**
     * If true, the reduced model will use the transition of a Markov
     * Chain that describes the continual planning approach that replans
     * after k exceptions occur. More details can be found in:
     *
     *      http://anytime.cs.umass.edu/shlomo/papers/PZicaps14.pdf
     *
     * This overrides useFullTransition_;
     */
    bool useContPlanEvaluationTransition_;

public:
    ReducedModel(mlcore::Problem *originalProblem,
                 ReducedTransitionConfig *config, int k) :
        originalProblem_(originalProblem),
        config_(config),
        k_(k),
        useFullTransition_(false),
        useContPlanEvaluationTransition_(false)
    {
        s0 = new ReducedState(originalProblem_->initialState(), 0, this);
        this->addState(s0);
        actions_ = originalProblem->actions();
        gamma_ = originalProblem_->gamma();
    }

    virtual ~ReducedModel() {}

    void k(int value) { k_ = value; }

    void useFullTransition(bool value) { useFullTransition_ = value; }

    void useContPlanEvaluationTransition(bool value)
    {
        useContPlanEvaluationTransition_ = value;
    }

    /**
     * Computes the true expected cost of the continual planning approach
     * that replans after k exceptions according to the given reduced model.
     * The cost is computed as in:
     *
     *      http://anytime.cs.umass.edu/shlomo/papers/PZicaps14.pdf
     *
     * The method receives a solver to compute the continual plan.
     *
     * @param reducedModel the reduced model that induces the plan.
     * @param solver the algorithm used to generate the plan. It must be
     *              allocated before calling this function with the
     *              given reduced model.
     * @return the expected cost of the continual plan.
     */
    static double evaluateContinualPlan(ReducedModel* reducedModel,
                                          mlsolvers::Solver* solver);

    /**
     * Implements a reduced transition model for this problem according to
     * the stored configuration.
     */
    virtual std::list<mlcore::Successor>
    transition(mlcore::State* s, mlcore::Action *a);

    /**
     * Overrides method from Problem.
     */
    virtual bool goal(mlcore::State* s) const
    {
        ReducedState* rs = (ReducedState* ) s;
        return originalProblem_->goal(rs->originalState());
    }

    /**
     * Overrides method from Problem.
     */
    virtual double cost(mlcore::State* s, mlcore::Action* a) const
    {
        ReducedState* rs = (ReducedState* ) s;
        return originalProblem_->cost(rs->originalState(), a);
    }

    /**
     * Overrides method from Problem.
     */
    virtual bool applicable(mlcore::State* s, mlcore::Action* a) const
    {
        ReducedState* rs = (ReducedState* ) s;
        return originalProblem_->applicable(rs->originalState(), a);
    }
};

}


#endif // MDPLIB_REDUCEDMODEL_H
