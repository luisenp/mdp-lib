#ifndef MDPLIB_REDUCEDMODEL_H
#define MDPLIB_REDUCEDMODEL_H

#include <cfloat>
#include <list>

#include "../domains/WrapperProblem.h"

#include "../solvers/Solver.h"

#include "../Action.h"
#include "../Problem.h"
#include "../State.h"

#include "ReducedHeuristicWrapper.h"
#include "ReducedState.h"
#include "ReducedTransition.h"

namespace mlreduced
{

/**
 * This class implements the reduced model framework described in
 * http://anytime.cs.umass.edu/shlomo/papers/PZicaps14.pdf.
 */
class ReducedModel : public mlcore::Problem
{
private:

    /*
     * If true, then the destructor can be called safely.
     */
    bool clean_ = false;

    /*
     * If true, k is increased when re-planning if all states are solved
     */
    bool increase_k_ = false;

    /*
     * The time it takes to execute an action (default to DBL_MAX).
     * It is used to account for the time using for planning not
     * concurrently with execution in the trial() method.
     */
    double kappa_ = DBL_MAX;

    /*
     * Triggers a re-planning computation using this reduced model
     * an returns the time spent planning.
     * Proactive re-planning plans for the set of successors of the given
     * state under the full transition model, with exception counters
     * set to 0.
     * More details can be found in
     * http://anytime.cs.umass.edu/shlomo/papers/PZicaps14.pdf
     *
     * @param solver The MDP solver used for re-planning.
     * @param nextState The state that triggered re-planning.
     * @param proactive If true, then the plan will be created for the
     *                 successors of nextState with j=0. Otherwise the
     *                 plan is found for nextState directly.
     * @param wrapperProblem A WrapperProblem object used for setting new
     *                      successors for the pro-active planning approach.
     * @return The time spent planning (in seconds).
     *
     */
    double triggerReplan(mlsolvers::Solver& solver,
                         ReducedState* nextState,
                         bool proactive,
                         WrapperProblem* wrapperProblem);

    /*
     * Re-planning trigger handler used with the anytime version of MKL-replan.
     */
    double triggerReplanAnytime(mlsolvers::Solver& solver,
                                ReducedState* nextState,
                                bool proactive,
                                WrapperProblem* wrapperProblem);


protected:
    /**
     * The problem for which the reduced model is constructed.
     */
    mlcore::Problem* originalProblem_;

    /**
     * An object describing the outcomes to be considered primary for
     * each state, action pair.
     */
    ReducedTransition* reducedTransition_;

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
    ReducedModel(mlcore::Problem* originalProblem,
                 ReducedTransition* reducedTransition,
                 int k,
                 double kappa = DBL_MAX) :
            originalProblem_(originalProblem),
            reducedTransition_(reducedTransition),
            k_(k),
            useFullTransition_(false),
            useContPlanEvaluationTransition_(false),
            clean_(false),
            kappa_(kappa) {
        s0 = new ReducedState(originalProblem_->initialState(), k, this);
        this->addState(s0);
        actions_ = originalProblem->actions();
        gamma_ = originalProblem_->gamma();
        if (reducedTransition_ == nullptr)
            useFullTransition_ = true;
    }

    virtual ~ReducedModel() {
        assert(clean_);
    }

    /**
     * Returns the original problem.
     */
    mlcore::Problem* originalProblem() { return originalProblem_; }

    /**
     * Cleans up any data that may affect the original model when
     * the reduced model is destroyed. For example, the reference to
     * the actions in the original model is released, so that
     * destroying the actions in this model does not destroy the
     * original ones.
     */
    void cleanup() {
        actions_ = std::list<mlcore::Action*> ();
        clean_ = true;
    }

    /**
     * Sets the maximum number of exceptions before considering only
     * primary outcomes in the transition function.
     */
    void k(int value) { k_ = value; }

    /**
     * Returns the maximum number of exceptions before considering only
     * primary outcomes in the transition function.
     */
    int k() { return k_; }

    void increaseK(bool value) { increase_k_ = value; }

    void useFullTransition(bool value) {
        useFullTransition_ = value | (reducedTransition_ == nullptr);
    }

    /**
     * Sets whether the full transition model should be used or not.
     */
    void useContPlanEvaluationTransition(bool value)
    {
        useContPlanEvaluationTransition_ = value;
        if (value)
            useFullTransition_ = false;

    }

    /**
     * Computes the cost of executing a trial of the continual planning
     * approach based on this reduced model.
     *
     * @param solver An MDP solver to compute the online policy.
     * @param wrapperProblem A WrapperProblem for the reduced model, to be
     *                      used for setting new successors during
     *                      re-planning. The internal problem of this wrapper
     *                      must correspond to the reduced model.
     * @param maxPlanningTime A pointer to store the maximum amount of time
     *                        used for planning during the trials.
     * @return A pair that contains the cost of the trial and the time
     *        spent planning (not-concurrently with execution)
     *        during this trial (in seconds).
     */
    std::pair<double, double> trial(mlsolvers::Solver & solver,
                                    WrapperProblem* wrapperProblem,
                                    double* maxPlanningTime = nullptr);

    /**
     * Same as [trial], but uses the anytime version of MKL-replan instead.
     */
    std::pair<double, double> trialAnytime(mlsolvers::Solver & solver,
                                           WrapperProblem* wrapperProblem,
                                           double* maxPlanningTime = nullptr);

    /**
     * Computes the true expected cost of the continual planning approach
     * that replans after k exceptions according to the given reduced model.
     * The cost is computed as in:
     *
     *      http://anytime.cs.umass.edu/shlomo/papers/PZicaps14.pdf
     *
     * @param reducedModel the reduced model that induces the plan.
     * @return the expected cost of the continual plan.
     */
    static double evaluateMarkovChain(ReducedModel* reducedModel);

    /**
     * Computes the true expected cost of the continual planning approach
     * implied by this reduced model, using Monte Carlo evaluation.
     *
     * @param numSims The number of trials to use for computing the expected
     *               cost.
     */
    double evaluateMonteCarlo(int numTrials);

    /**
     * Finds the best reduced model that can be created for the given problem
     * out of the list of given reduced transition functions.
     *
     * @param originalProblem the problem to be reduced.
     * @param reducedTransitions the transitions to try.
     * @param k the bound on the number of exceptions.
     * @param heuristic the heuristic to use.
     * @return the best reduced transition function out of the given list.
     *        The reductions are evaluated using evaluateContinualPlan.
     */
    static ReducedTransition* getBestReduction(
        mlcore::Problem *originalProblem,
        std::list<ReducedTransition *> reducedTransitions,
        int k,
        ReducedHeuristicWrapper* heuristic);


    /**
     * Returns true if successor is an exception for state when the give action
     * is executed.
     */
    bool isException(
        mlcore::State* state, mlcore::State* successor, mlcore::Action* action);

    /**
     * Implements a reduced transition model for this problem according to
     * the stored reduced transition.
     */
    virtual std::list<mlcore::Successor>
    transition(mlcore::State* s, mlcore::Action *a);

    /**
     * Overrides method from Problem.
     */
    virtual bool goal(mlcore::State* s) const {
        ReducedState* rs = static_cast<ReducedState*> (s);
        return originalProblem_->goal(rs->originalState());
    }

    /**
     * Overrides method from Problem.
     */
    virtual double cost(mlcore::State* s, mlcore::Action* a) const {
        ReducedState* rs = static_cast<ReducedState*> (s);
        return originalProblem_->cost(rs->originalState(), a);
    }

    /**
     * Overrides method from Problem.
     */
    virtual bool applicable(mlcore::State* s, mlcore::Action* a) const {
        ReducedState* rs = static_cast<ReducedState*> (s);
        return originalProblem_->applicable(rs->originalState(), a);
    }
};

}


#endif // MDPLIB_REDUCEDMODEL_H
