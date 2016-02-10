#ifndef MDPLIB_MOGRIDWORLDPROBLEM_H
#define MDPLIB_MOGRIDWORLDPROBLEM_H

#include <vector>

#include "../../Problem.h"
#include "../../Action.h"
#include "../../State.h"
#include "../../domains/gridworld/GridWorldProblem.h"
#include "../../domains/gridworld/GridWorldAction.h"
#include "../MObjProblem.h"
#include "../domains/MOGridWorldState.h"

namespace mlmobj
{

#define COST_DOWN_2 10

class MOGridWorldProblem : public MOProblem
{
private:
    int width_;
    int height_;
    int x0_;
    int y0_;
    double actionCost_;
    std::vector<PairDoubleMap> goals_;
    mlcore::State* absorbing;

    void addSuccessor(MOGridWorldState* state, std::list<mlcore::Successor>& successors, int val,
                      int limit, int newx, int newy, double prob);

    void addAllActions();

public:

    using MOProblem::goal;

    /**
     * Constructs a grid world with the specified width, height, goal states,
     * initial state (x0,y0). Heuristic is initialized to all-zero heuristic.
     */
    MOGridWorldProblem(int width, int height, int x0, int y0,
                         std::vector<PairDoubleMap>& goals, int size, double actionCost);

    virtual ~MOGridWorldProblem() {}

    /**
     * Returns the cost of actions on this grid world.
     *
     * @return the cost of actions.
     */
    double actionCost() { return actionCost_; }

    /**
     * Returns the goals associated with this problem.
     *
     * @return the goals of the problem.
     */
    std::vector<PairDoubleMap> & goals() { return goals_; }

    /**
     * Overrides method from MOProblem.
     */
    virtual double cost(mlcore::State* s, mlcore::Action* a, int i) const;

    /**
     * Overrides method from MOProblem.
     */
    virtual std::list<mlcore::Successor> transition(mlcore::State* s, mlcore::Action* a, int index);

    /**
     * Overrides method from MOProblem.
     */
    virtual bool goal(mlcore::State* s, int index) const;

    /**
     * Overrides method from MOProblem.
     */
    virtual bool applicable(mlcore::State* s, mlcore::Action* a) const;
};



}
#endif // MDPLIB_MOGRIDWORLDPROBLEM_H
