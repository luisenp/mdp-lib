#ifndef METAREASONINGPROBLEM_H
#define METAREASONINGPROBLEM_H

#include "../../problem.h"
#include "../../state.h"
#include "../../action.h"
#include "../../util/general.h"

#include "MetareasoningSimulator.h"

class MetareasoningState;

/**
 * This class represents a metareasoning problem based on Value Iteration.
 *
 * For a given MDP = <S,A,T,C> we have:
 *
 * The metareasoning state space is S x {0,...,I}, where I is the number of
 * iterations required to solve this problem using VI.
 *
 * The metareasoning action space is {PLAN, ACT}.
 *
 * The metareasoning transition function is defined as follows:
 *    T_meta( (s',i') | (s,i), ACT ) = T(s' | s, GREEDY) for i' = i + tau(s, GREEDY),
 *        where tau is a function that represents the number of computational
 *        time steps required to perform a in s, and GREEDY is the action chosen by
 *        VI for state s at the i-th time step.
 *    T_meta( (s,i+1) | (s,i), PLAN) = 1.0, for i' = i + tau(s, PLAN)
 *    0 otherwise.
 *
 * The cost function is defined as follows:
 *    C_meta( (s,i), ACT) = C(s, GREEDY)
 *    C_meta( (s,i), PLAN) = c_p
 *        where c_p represents the cost of planning.
 *
 */
class MetareasoningProblem : public mlcore::Problem
{

private:

    /* The simulator for the problem on which metareasoning will be applied. */
    mlsolvers::MetareasoningSimulator* simulator_;

public:
    MetareasoningProblem(mlsolvers::MetareasoningSimulator* simulator);

    virtual ~MetareasoningProblem() { }

    /**
     * Returns an action chosen greedily according to the values estimated by VI for
     * s.baseState() at iteration s.iteration().
     *
     * @param s the metareasoning state for which the greedy action will be computed.
     * @return an action chosen greedily on the current VI values for state s.
     */
    mlcore::Action* getGreedyActionForStateValues(MetareasoningState* s) const;

    /**
     * Overrides method from Problem.
     */
    virtual bool goal(mlcore::State* s) const;

    /**
     * Overrides method from Problem.
     */
    virtual std::list<mlcore::Successor> transition(mlcore::State* s,
                                                     mlcore::Action* a);

    /**
     * Overrides method from Problem.
     */
    virtual double cost(mlcore::State* s, mlcore::Action* a) const;

    /**
     * Overrides method from Problem.
     */
    virtual bool applicable(mlcore::State* s, mlcore::Action* a) const;
};

#endif // METAREASONINGPROBLEM_H
