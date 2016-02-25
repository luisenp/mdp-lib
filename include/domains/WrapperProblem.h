#ifndef MDPLIB_WRAPPERPROBLEM_H
#define MDPLIB_WRAPPERPROBLEM_H

#include <cassert>

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

    /*
     * A state whose successors can be changed on-the-fly.
     */
    DummyState* dummyState_;

    /*
     * An abosrbing state that transitions to itself at a cost of 0.
     */
    mlcore::State* absorbing_;

    /*
     * The only applicable action in the dummy state.
     */
    mlcore::Action* dummyAction_;

    /*
     * The internal problem this is wrapping.
     */
    mlcore::Problem* problem_;

    /*
     * A set of states that replace the original states in problem_.
     */
     mlcore::StateSet* overrideStates_ = nullptr;

    /*
     * A set of goals that replace the original goal in problem_.
     */
     mlcore::StateSet* overrideGoals_ = nullptr;

     /*
      * If true, then the destructor can be called safely.
      */
      bool clean_ = false;

public:

    WrapperProblem(mlcore::Problem* problem) : clean_(false)
    {
        dummyState_ = new DummyState();
        absorbing_ = new DummyState();
        this->addState(dummyState_);
        this->addState(absorbing_);
        setNewProblem(problem);
    }

    virtual ~WrapperProblem()
    {
        // Method free up must be called before the destructor.
        assert(clean_);
        // No need to delete dummy because the parent constructor will delete it.
    }

    /**
     * Changes the problem this wrapper is wrapping.
     *
     * @param problem The new problem to wrap.
     */
    void setNewProblem(mlcore::Problem* problem)
    {
        problem_ = problem;
        s0 = problem_->initialState();
        actions_ = problem->actions();
        dummyAction_ = actions_.front();
        heuristic_ = problem->heuristic();
        if (overrideStates_ != nullptr)
            overrideStates_->clear();
        if (overrideGoals_ != nullptr)
            overrideGoals_->clear();
    }

    mlcore::Problem* problem() { return problem_; }

    DummyState* dummyState() { return dummyState_; }

    void setDummyAction(mlcore::Action* action) { dummyAction_ = action; }

    /**
     * This method makes sure the destructor doesn't destroy the
     * states of the original problem.
     */
    void cleanup()
    {
        if (overrideStates_ != nullptr) {
            overrideStates_->clear();
            overrideStates_->insert(dummyState_);
        }
        actions_ = std::list<mlcore::Action*> ();
        clean_ = true;
    }

    /**
     * Adds a set of goals that override the goal of the original problem.
     * The original goals remain unchanged, but they are not used as long
     * as there are override goals.
     *
     * @param newGoals The set of goals that will override the previous one.
     */
    void overrideGoals(mlcore::StateSet* newGoals)
    {
        overrideGoals_ = newGoals;
    }

    /**
     * Returns the set of goals that override the goal of the original
     * problem.
     */
    mlcore::StateSet* overrideGoals() { return overrideGoals_; }

    /**
     * Adds a goal to the set of override goals.
     * The original goals remain unchanged, but they are not used as long
     * as there are override goals.
     *
     * @param overrideGoal The goal to add.
     */
    void addOverrideGoal(mlcore::State* overrideGoal)
        { overrideGoals_->insert(overrideGoal); }

    /**
     * Clears the set of override goals.
     */
    void clearOverrideGoals() { overrideGoals_->clear(); }

    /**
     * Sets the given state as the initial state of the problem.
     */
    void setNewInitialState(mlcore::State* newInitialState)
    {
        s0 = newInitialState;
    }

    /**
     * Adds a set of states that override the states of the original problem.
     * The original states remain unchanged, but they are not used as long
     * as there are override states.
     *
     * @param newStates The set of states that will override the previous ones.
     */
    void overrideStates(mlcore::StateSet* newStates)
    {
        overrideStates_ = newStates;
    }

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
