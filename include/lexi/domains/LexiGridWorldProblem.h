#ifndef MDPLI_LEXIGRIDWORLDPROBLEM_H
#define MDPLI_LEXIGRIDWORLDPROBLEM_H

#include <vector>

#include "../../problem.h"
#include "../../action.h"
#include "../../state.h"
#include "../../domains/gridworld/GridWorldProblem.h"
#include "../../domains/gridworld/GridWorldState.h"
#include "../../domains/gridworld/GridWorldAction.h"
#include "../../lexi/lexi_problem.h"

namespace mllexi
{

class LexiGridWorldProblem : public LexiProblem
{
private:
    int width_;
    int height_;
    int x0_;
    int y0_;
    int numValueFunc_;
    double actionCost_;
    std::vector<PairDoubleMap> goals_;
    mlcore::State* absorbing;

    void addSuccessor(GridWorldState* state, std::list<mlcore::Successor>& successors, int val,
                      int limit, int newx, int newy, double prob);

    void addAllActions();

public:
    /**
     * Constructs a grid world with the specified width, height, goal states,
     * initial state (x0,y0). Heuristic is initialized to all-zero heuristic.
     */
    LexiGridWorldProblem(int width, int height, int x0, int y0,
                         std::vector<PairDoubleMap>& goals, double actionCost);

    virtual ~LexiGridWorldProblem() {}

    /**
     * Overrides method from LexiProblem.
     */
    virtual std::vector<double> lexiCost(mlcore::State* s, mlcore::Action* a) const;

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
