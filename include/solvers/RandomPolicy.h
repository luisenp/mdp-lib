#ifndef MDPLIB_RANDOMPOLICY_H
#define MDPLIB_RANDOMPOLICY_H

#include <vector>
#include <iostream>

#include "../Action.h"
#include "../State.h"

#include "Solver.h"

namespace mlsolvers
{

/**
 * A class representing a random policy.It provides a method to add
 * probabilistic actions for a given state, and one for retrieving, for a
 * given state, a random action according to the probability distribution of
 * actions associated to the given state.
 */
class RandomPolicy
{

private:
    mlcore::Problem* problem_;

    mlcore::StateIntMap stateIndex_;

    mlcore::StateDoubleMap values_;

    std::vector< std::vector <double> > policy_;

    int stateCounter_;

public:

    /**
     * Creates a random policy for a problem with the given number of states.
     *
     * @param problem the problem to which the policy is associated.
     * @param numStates the number of states in the problem.
     */
    RandomPolicy(mlcore::Problem* problem, int numStates) : problem_(problem)
    {
        policy_ = std::vector< std::vector <double> > (numStates);
        stateCounter_ = 0;
    }

    virtual ~RandomPolicy() {}

    /**
     * Associates the given state with the given vector representing a
     * probability distribution over actions. The order of the action is
     * assumed to be the same order returned by Problem::actions().
     *
     * @param s the state that will be associated with these set of actions.
     * @param actions the probability distribution over actions that will
     *                be associated with this state.
     */
    void addActionsState(mlcore::State* s, std::vector<double> actions);

    /**
     * Prints the policy to the given output stream.
     */
     void print(std::ostream& os);

    /**
     * Returns an action sampled from the probability distribution associated
     * with the given state.
     *
     * @param s the state for which the action is requested.
     */
    mlcore::Action* getRandomAction(mlcore::State* s);

    /**
     * Returns the value of the given state under this policy.
     *
     * @param s the state for which the value will be returned.
     * @return the value of the state under this policy.
     */
    double getValue(mlcore::State* s);
};


} // namespace mlsolvers

#endif // MDPLIB_RANDOMPOLICY_H
