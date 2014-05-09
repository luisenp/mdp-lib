#ifndef GRIDWORLDPROBLEM_H
#define GRIDWORLDPROBLEM_H

#include <unordered_set>
#include "../../problem.h"
#include "../../state.h"
#include "../../action.h"
#include "../../util/general.h"
#include "../../util/rational.h"

namespace gridworld
{
    const unsigned char UP = 0;
    const unsigned char DOWN = 1;
    const unsigned char LEFT= 2;
    const unsigned char RIGHT = 3;
}

class GridWorldProblem : public Problem
{
private:
    int width_;
    int height_;
    IntPairSet * goals_;

public:
    GridWorldProblem();

    /**
     * Constructs a grid world with the specified width, height and goal states
     */
    GridWorldProblem(int width, int height, IntPairSet * goals);

    ~GridWorldProblem();


    /**
     * Returns true if state s is a goal.
     */
    virtual bool goal(const State *s);

    /**
     * Returns a list of all succcessors when action a is applied in state s.
     */
    virtual std::list<Successor> transition(State *s, Action *a) ;

    /**
     * Returns the cost of applying action a in state s.
     */
    virtual Rational cost(State *s, Action *a);

    /**
     * Returns true if action a can be applied in state s.
     */
    virtual bool applicable(State *s, Action *a);
};

#endif // GRIDWORLDPROBLEM_H
