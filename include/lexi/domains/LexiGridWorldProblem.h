#ifndef MDPLI_LEXIGRIDWORLDPROBLEM_H
#define MDPLI_LEXIGRIDWORLDPROBLEM_H

#include <vector>

#include "../../problem.h"
#include "../../action.h"
#include "../../state.h"
#include "../../domains/gridworld/GridWorldProblem.h"
#include "../../domains/gridworld/GridWorldAction.h"
#include "../../lexi/lexi_problem.h"
#include "../../lexi/domains/LexiGridWorldState.h"

namespace mllexi
{

#define COST_DOWN_2 10

class LexiGridWorldProblem : public LexiProblem
{
private:
    int width_;
    int height_;
    int x0_;
    int y0_;
    double actionCost_;
    std::vector<PairDoubleMap> goals_;
    mlcore::State* absorbing;

    void addSuccessor(LexiGridWorldState* state, std::list<mlcore::Successor>& successors, int val,
                      int limit, int newx, int newy, double prob);

    void addAllActions();

public:

    using LexiProblem::goal;

    /**
     * Constructs a grid world with the specified width, height, goal states,
     * initial state (x0,y0). Heuristic is initialized to all-zero heuristic.
     */
    LexiGridWorldProblem(int width, int height, int x0, int y0,
                         std::vector<PairDoubleMap>& goals, int size, double actionCost);

    virtual ~LexiGridWorldProblem() {}

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
     * Overrides method from LexiProblem.
     */
    virtual double cost(mlcore::State* s, mlcore::Action* a, int i) const;

    /**
     * Overrides method from LexiProblem.
     */
    virtual std::list<mlcore::Successor> transition(mlcore::State* s, mlcore::Action* a, int index);

    /**
     * Overrides method from LexiProblem.
     */
    virtual bool goal(mlcore::State* s, int index) const;

    /**
     * Overrides method from LexiProblem.
     */
    virtual bool applicable(mlcore::State* s, mlcore::Action* a) const;
};



}
#endif // MDPLI_LEXIGRIDWORLDPROBLEM_H
