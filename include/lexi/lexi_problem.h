#ifndef MDPLIB_LEXIPROBLEM_H
#define MDPLIB_LEXIPROBLEM_H

#include "../problem.h"
#include "../state.h"
#include "../action.h"


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
private:
public:
    LexiProblem();
    virtual ~LexiProblem();

    /**
     * Lexicographical cost function for the problem.
     *
     * Returns a vector with the cost of applying the given action to the given state
     * under each of the value functions, ordered by decreasing order of preference.
     *
     * @return The vector with the costs of applying action the given action to
     * the given state.
     */
    virtual std::vector<double> lexiCost(mlcore::State* s, mlcore::Action* a) const = 0;

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
    virtual bool goal(mlcore::State* s) const {}

    /**
     * Overrides method from mlcore::Problem.
     */
    virtual std::list<mlcore::Successor> transition(mlcore::State* s, mlcore::Action* a) {}

    /**
     * Overrides method from mlcore::Problem.
     */
    virtual double cost(mlcore::State* s, mlcore::Action* a) const {}

    /**
     * Overrides method from mlcore::Problem.
     */
    virtual bool applicable(mlcore::State* s, mlcore::Action* a) const;

};

}

#endif // MDPLIB_LEXIPROBLEM_H
