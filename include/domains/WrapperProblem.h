#ifndef MDPLIB_WRAPPERPROBLEM_H
#define MDPLIB_WRAPPERPROBLEM_H

#include "../Problem.h"
#include "../State.h"

#include "DummyState.h"

/**
 * This class wraps a mlcore::Problem instance and adds to it a DummyState whose
 * successors can be defined externally.
 */
class WrapperProblem : public mlcore::Problem
{
private:

    DummyState* dummyState_;

    mlcore::Problem* problem_;
    mlcore::Action* dummyAction_;

    /*
     * A set of states that replace the original states in problem_.
     */
     mlcore::StateSet overrideStates_;

    /*
     * A set of goals that replace the original goal in problem_.
     */
     mlcore::StateSet overrideGoals_;

public:

    WrapperProblem(mlcore::Problem* problem)
    {
        dummyState_ = new DummyState();
        setNewProblem(problem);
    }

    virtual ~WrapperProblem() { }

    DummyState* dummyState() { return dummyState_; }

    void setDummyAction(mlcore::Action* action) { dummyAction_ = action; }

    void overrideGoals(mlcore::StateSet& newGoals)
        { overrideGoals_ = newGoals; }

    void addOverrideGoal(mlcore::State* overrideGoal)
        { overrideGoals_.insert(overrideGoal); }

    void clearOverrideGoals() { overrideGoals_.clear(); }

    void setNewProblem(mlcore::Problem* problem)
    {
        problem_ = problem;
        s0 = problem_->initialState();
        actions_ = problem->actions();
        heuristic_ = problem->heuristic();
        overrideStates_.clear();
        overrideGoals_.clear();
    }

    void overrideStates(mlcore::StateSet& value) { overrideStates_ = value; }

    /**
     * Overrides method from Problem.
     */
    virtual mlcore::StateSet& states();

    /**
     * Overrides method from Problem.
     */
    virtual bool goal(mlcore::State* s) const;

    /**
     * Overrides method from Problem.
     */
    virtual std::list<mlcore::Successor>
    transition(mlcore::State* s, mlcore::Action* a);

    /**
     * Overrides method from Problem.
     */
    virtual double cost(mlcore::State* s, mlcore::Action* a) const;

    /**
     * Overrides method from Problem.
     */
    virtual bool applicable(mlcore::State* s, mlcore::Action* a) const;
};

#endif // MDPLIB_WRAPPERPROBLEM_H
