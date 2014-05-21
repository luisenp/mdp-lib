#ifndef MPDLIB_GRIDWORLDPROBLEM_H
#define MPDLIB_GRIDWORLDPROBLEM_H

#include <unordered_set>

#include "GridWorldState.h"
#include "../../problem.h"
#include "../../state.h"
#include "../../action.h"
#include "../../util/general.h"
#include "../../util/rational.h"

namespace gridworld
{
    const unsigned char UP = 0;
    const unsigned char DOWN = 1;
    const unsigned char LEFT = 2;
    const unsigned char RIGHT = 3;
}

class GridWorldProblem : public Problem
{
private:
    int width_;
    int height_;
    int x0_;
    int y0_;
    IntPairSet* goals_;

    void addSuccessor(GridWorldState* state, std::list<Successor>& successors, int val,
                      int limit, int newx, int newy, Rational prob);

public:
    GridWorldProblem();

    /**
     * Constructs a grid world with the specified width, height, goal states
     * and initial state (x0,y0).
     */
    GridWorldProblem(int width, int height, int x0, int y0, IntPairSet* goals);

    ~GridWorldProblem();

    /**
     * Returns true if state s is a goal.
     */
    virtual bool goal(State* s) const;

    /**
     * Returns a list of all succcessors when action a is applied in state s.
     */
    virtual std::list<Successor> transition(State* s, Action* a);

    /**
     * Returns the cost of applying action a in state s.
     */
    virtual Rational cost(State* s, Action* a) const;

    /**
     * Returns true if action a can be applied in state s.
     */
    virtual bool applicable(State* s, Action* a) const;

    const State* getInitialState() const;

    virtual void generateAll();
};

#endif // MPDLIB_GRIDWORLDPROBLEM_H
