#ifndef MDPLIB_AODET_H
#define MDPLIB_AODET_H

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../Problem.h"
#include "../State.h"

/**
 * This class can be used to create the all outcomes determinization of a
 * problem.
 */
class AllOutcomesDeterminization : public mlcore::Problem {

private:
    /* The original problem. */
    mlcore::Problem* originalProblem_;

    /* Maps states in the original problem to states in this problem. */
    mlcore::StateIntMap stateIndexMap_;

    /*
     * Represents the transition graph of the all outcomes determinization.
     * transitionGraph_[i] stores the actions/successors of state with index i.
     * Then j belongs to the keys of transitionGraph_[i] if the action is
     * applicable. In that case k = transitionGraph_[i][j] is the index of
     * the state resulting from this transition.
     */
    std::vector< std::unordered_map<int, int> > transitionGraph_;

    /* Costs of the actions in the determinization. */
    std::vector<double> actionCosts_;

    /*
     * Stores all the states in the order implied by their indices, so that
     * they can be retrieved easily.
     */
    std::vector<mlcore::State*> allStates_;

    /*
     * Stores all the actions in a vector so that they can be easily indexed.
     */
    std::vector<mlcore::Action*> actionsVector_;

public:

    AllOutcomesDeterminization(mlcore::Problem* problem);

    virtual ~AllOutcomesDeterminization() {}

    /**
     * Returns the list of actions that are applicable in with this state.
     */
    std::list<mlcore::Action*> actions(mlcore::State* s) const;

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
     * If a state is part of overrideGoals_, this method will return the
     * current estimated cost of the state as the cost of the action.
     */
    virtual double cost(mlcore::State* s, mlcore::Action* a) const;

    /**
     * Overrides method from Problem.
     */
    virtual bool applicable(mlcore::State* s, mlcore::Action* a) const;
};

#endif // MDPLIB_AODET_H
