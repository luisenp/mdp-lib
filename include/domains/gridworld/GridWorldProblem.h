#ifndef MPDLIB_GRIDWORLDPROBLEM_H
#define MPDLIB_GRIDWORLDPROBLEM_H

#include <unordered_set>

#include "GridWorldState.h"

#include "../../problem.h"
#include "../../state.h"
#include "../../action.h"
#include "../../util/general.h"
#include "../../util/rational.h"

class GWManhattanHeuristic;

namespace gridworld
{
    const unsigned char UP = 0;
    const unsigned char DOWN = 1;
    const unsigned char LEFT = 2;
    const unsigned char RIGHT = 3;
}

/**
 * A class representing a grid world problem as described in AIAMA 3rd Edition.
 */
class GridWorldProblem : public Problem
{
    friend class GWManhattanHeuristic;

private:
    int width_;
    int height_;
    int x0_;
    int y0_;
    PairRationalMap* goals_;
    State* absorbing;

    void addSuccessor(GridWorldState* state, std::list<Successor>& successors, int val,
                      int limit, int newx, int newy, Rational prob);

    void addAllActions();
public:
    /**
     * Default constructor.
     *
     * Creates a grid world with a single goal state (0,0) an all-zero heuristic.
     */
    GridWorldProblem();

    /**
     * Constructs a grid world with the specified width, height, goal states,
     * initial state (x0,y0). Heuristic is initialized to all-zero heuristic.
     */
    GridWorldProblem(int width, int height, int x0, int y0, PairRationalMap* goals);

    /**
     * Constructs a grid world with the specified width, height, goal states,
     * initial state (x0,y0) and heuristic.
     */
    GridWorldProblem(int width, int height, int x0, int y0, PairRationalMap* goals, Heuristic* h);

    ~GridWorldProblem();

    virtual bool goal(State* s) const;
    virtual std::list<Successor> transition(State* s, Action* a);
    virtual Rational cost(State* s, Action* a) const;
    virtual bool applicable(State* s, Action* a) const;
};

#endif // MPDLIB_GRIDWORLDPROBLEM_H
