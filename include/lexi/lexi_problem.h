#ifndef MDPLIB_LEXIPROBLEM_H
#define MDPLIB_LEXIPROBLEM_H

#include "../problem.h"
#include "../state.h"
#include "../action.h"

#include "../util/general.h"


namespace mllexi
{

/**
 * An abstract class for Stochastic Shortest Path Problems with lexicographical
 * preferences.
 *
 * This class inherits the core methods from mlcore::Problem and provides three
 * additional methods. One method, lexiCost, provides a cost function that
 * returns a vector of values ordered in decreasing order of preference. The other
 * two methods are modified versions of the goal condition and transition function
 * that receive an index indicating with respect to which value function the
 * operation must be performed.
 */
class LexiProblem : public mlcore::Problem
{
protected:
    int size_;
    double slack_;

public:
    LexiProblem() : slack_(0.0) { }
    virtual ~LexiProblem() {}

    /**
     * Returns the number of value functions for this lexicographical problem.
     *
     * @return the number of value functions for this lexicographical problem.
     */
    int size() const { return size_; }

    /**
     * Returns the slack to use for this lexicographical problem.
     *
     * @return the slack of the problem.
     */
    int slack() const { return slack_; }

    /**
     * Lexicographical cost function for the problem.
     *
     * Returns the cost of applying the given action to the given state
     * according to the i-th value function in the lexicographical
     * order of the problem.
     *
     * @return the cost of applying action the given action to the given state according
     *        to the specified value function.
     */
    virtual double cost(mlcore::State* s, mlcore::Action* a, int i) const = 0;

    /**
     * Lexicographical goal check.
     *
     * Checks if the state given as parameter is a goal or not under the value function
     * specified by the given index.
     *
     * @return true if the given state is a goal under the given value function.
     */
    virtual bool goal(mlcore::State* s, int index) const = 0;

    /**
     * Lexicographical transition function for the problem.
     *
     * Returns a list with all successors of the given state when the given action
     * is applied. The index indicates with respect to which value function it the
     * transition to be computed. Note that this index should only modify the behavior
     * transition function for goal states, so that they become terminal at the right
     * level.
     *
     * @return A list of succcessors of the given state after applying the
     *        given action.
     */
    virtual
    std::list<mlcore::Successor> transition(mlcore::State* s, mlcore::Action* a, int index) = 0;

    /**
     * Overrides method from mlcore::Problem.
     */
    virtual bool applicable(mlcore::State* s, mlcore::Action* a) const = 0;

    /**
     * Overrides method from mlcore::Problem.
     */
    virtual bool goal(mlcore::State* s) const
    {
        return goal(s, 0);
    }

    /**
     * Overrides method from mlcore::Problem.
     */
    virtual std::list<mlcore::Successor> transition(mlcore::State* s, mlcore::Action* a)
    {
        return transition(s, a, 0);
    }

    /**
     * Overrides method from mlcore::Problem.
     */
    virtual double cost(mlcore::State* s, mlcore::Action* a) const
    {
        return cost(s, a, 0);
    }


};

}

#endif // MDPLIB_LEXIPROBLEM_H
