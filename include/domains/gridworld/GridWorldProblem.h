#ifndef MPDLIB_GRIDWORLDPROBLEM_H
#define MPDLIB_GRIDWORLDPROBLEM_H

#include <unordered_set>

#include "GridWorldState.h"

#include "../../problem.h"
#include "../../state.h"
#include "../../action.h"
#include "../../util/general.h"

class GWManhattanHeuristic;

namespace gridworld
{
    const unsigned char UP = 0;
    const unsigned char DOWN = 1;
    const unsigned char LEFT = 2;
    const unsigned char RIGHT = 3;
}

/**
 * A class representing a grid world problem as described
 * in AIAMA 3rd Edition.
 */
class GridWorldProblem : public mlcore::Problem
{
    friend class GWManhattanHeuristic;

private:
    int width_;
    int height_;
    int x0_;
    int y0_;
    double actionCost_;
    PairDoubleMap* goals_;
    mlcore::State* absorbing;

    void addSuccessor(GridWorldState* state,
                      std::list<mlcore::Successor>& successors,
                      int val,
                      int limit,
                      int newx,
                      int newy,
                      double prob);

    void addAllActions();
public:
    /**
     * Default constructor.
     *
     * Creates a grid world with a single goal state (0,0)
     * and all-zero heuristic.
     */
    GridWorldProblem();

    /**
     * Constructs a grid world with the specified width, height,
     * goal states, initial state (x0,y0).
     * Heuristic is initialized to all-zero heuristic.
     */
    GridWorldProblem(int width, int height, int x0, int y0,
                     PairDoubleMap* goals, double actionCost);

    /**
     * Constructs a grid world with the specified width, height,
     * goal states, initial state (x0,y0) and heuristic.
     */
    GridWorldProblem(int width, int height, int x0, int y0,
                     PairDoubleMap* goals, mlcore::Heuristic* h);

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

#endif // MPDLIB_GRIDWORLDPROBLEM_H
