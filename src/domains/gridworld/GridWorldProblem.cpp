#include "../../../include/domains/gridworld/GridWorldProblem.h"
#include "../../../include/domains/gridworld/GridWorldState.h"
#include "../../../include/domains/gridworld/GridWorldAction.h"

GridWorldProblem::GridWorldProblem() : width_(0), height_(0), goals_(0) {}

GridWorldProblem::GridWorldProblem(int width, int height, IntPairSet * goals)
                                   : width_(width), height_(height), goals_(goals) { }

GridWorldProblem::~GridWorldProblem() { }

bool GridWorldProblem::goal(const State *s)
{
    GridWorldState *gws = (GridWorldState *) s;
    std::pair<int,int> pos(gws->x(),gws->y());
    return goals_->find(pos) != goals_->end();
}

std::list<Successor> GridWorldProblem::transition(State *s, Action *a)
{
    GridWorldState *state = (GridWorldState *) s;
    GridWorldAction *action = (GridWorldAction *) a;

    std::list<Successor> successors;
    if (action->dir() == gridworld::UP) {
        if (state->y() == height_ - 1) {
            successors.push_front(Successor(state, Rational(0.8)));
        } else {
            const GridWorldState *next = new GridWorldState(state->x(), state->y() + 1);
            successors.push_front(Successor(getState(next), Rational(0.1)));
        }
        if  (state->x() == 0) {
            successors.push_front(Successor(state, Rational(0.1)));
        } else {
            const GridWorldState *next = new GridWorldState(state->x() - 1, state->y());
            successors.push_front(Successor(getState(next), Rational(0.1)));
        }
        if  (state->x() == width_ - 1) {
            successors.push_front(Successor(state, Rational(0.1)));
        } else {
            const GridWorldState *next = new GridWorldState(state->x() + 1, state->y());
            successors.push_front(Successor(getState(next), Rational(0.1)));
        }
    }

    return successors;
}

Rational GridWorldProblem::cost(State *s, Action *a)
{
    return Rational(0);
}

bool GridWorldProblem::applicable(State *s, Action *a)
{
    return false;
}
