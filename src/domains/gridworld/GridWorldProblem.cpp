#include "../../../include/domains/gridworld/GridWorldProblem.h"
#include "../../../include/domains/gridworld/GridWorldState.h"
#include "../../../include/domains/gridworld/GridWorldAction.h"
#include "../../../include/util/rational.h"

GridWorldProblem::GridWorldProblem() : width_(0), height_(0), x0_(0), y0_(0), goals_(0) {}

GridWorldProblem::GridWorldProblem(int width, int height, int x0, int y0, IntPairSet * goals)
                                   : width_(width), height_(height),
                                      x0_(x0), y0_(y0), goals_(goals)
{
    const State* init = new GridWorldState(x0_, y0_);
    s0 = this->getState(init);
}

GridWorldProblem::~GridWorldProblem() { }

bool GridWorldProblem::goal(const State* s) const
{
    GridWorldState* gws = (GridWorldState *) s;
    std::pair<int,int> pos(gws->x(),gws->y());
    return goals_->find(pos) != goals_->end();
}

std::list<Successor> GridWorldProblem::transition(State *s, Action *a)
{
    GridWorldState* state = (GridWorldState *) s;
    GridWorldAction* action = (GridWorldAction *) a;

    std::list<Successor> successors;
    if (action->dir() == gridworld::UP) {
        if (state->y() == height_ - 1) {
            successors.push_front(Successor(state, Rational(0.8)));
        } else {
            const GridWorldState *next = new GridWorldState(state->x(), state->y() + 1);
            successors.push_front(Successor(this->getState(next), Rational(0.1)));
        }
        if  (state->x() == 0) {
            successors.push_front(Successor(state, Rational(0.1)));
        } else {
            const GridWorldState *next = new GridWorldState(state->x() - 1, state->y());
            successors.push_front(Successor(this->getState(next), Rational(0.1)));
        }
        if  (state->x() == width_ - 1) {
            successors.push_front(Successor(state, Rational(0.1)));
        } else {
            const GridWorldState *next = new GridWorldState(state->x() + 1, state->y());
            successors.push_front(Successor(this->getState(next), Rational(0.1)));
        }
    }

    return successors;
}

Rational GridWorldProblem::cost(State* s, Action* a) const
{
    return Rational(0);
}

bool GridWorldProblem::applicable(State* s, Action* a) const
{
    return false;
}

const State* GridWorldProblem::getInitialState() const
{
    return s0;
}
