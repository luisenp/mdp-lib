#include "../../../include/domains/gridworld/GridWorldProblem.h"
#include "../../../include/domains/gridworld/GridWorldState.h"
#include "../../../include/domains/gridworld/GridWorldAction.h"
#include "../../../include/util/rational.h"

GridWorldProblem::GridWorldProblem() : width_(0), height_(0), x0_(0), y0_(0), goals_(0)
{
    Action* up = new GridWorldAction(gridworld::UP);
    Action* down = new GridWorldAction(gridworld::DOWN);
    Action* left = new GridWorldAction(gridworld::LEFT);
    Action* right = new GridWorldAction(gridworld::RIGHT);
    actions.push_front(up);
    actions.push_front(down);
    actions.push_front(left);
    actions.push_front(right);
}

GridWorldProblem::GridWorldProblem(int width, int height, int x0, int y0, IntPairSet * goals)
                                   : width_(width), height_(height),
                                      x0_(x0), y0_(y0), goals_(goals)
{
    State* init = new GridWorldState(x0_, y0_);
    s0 = this->getState(init);
    Action* up = new GridWorldAction(gridworld::UP);
    Action* down = new GridWorldAction(gridworld::DOWN);
    Action* left = new GridWorldAction(gridworld::LEFT);
    Action* right = new GridWorldAction(gridworld::RIGHT);
    actions.push_front(up);
    actions.push_front(down);
    actions.push_front(left);
    actions.push_front(right);
}

GridWorldProblem::~GridWorldProblem()
{
    for (State* state : states)
        delete (state);
    for (Action* action : actions)
        delete (action);
}

bool GridWorldProblem::goal(State* s) const
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

    if (goal(s)) {
        successors.push_front(Successor(state, Rational(1)));
        return successors;
    }

    if (action->dir() == gridworld::UP) {
        addSuccessor(state, successors, height_ - 1, state->y(),
                     state->x(), state->y() + 1, Rational(8, 10));

        addSuccessor(state, successors, state->x(), 0,
                     state->x() - 1, state->y(), Rational(1, 10));

        addSuccessor(state, successors, width_ - 1, state->x(),
                     state->x() + 1, state->y(), Rational(1, 10));
    } else if (action->dir() == gridworld::DOWN) {
        addSuccessor(state, successors, state->y(), 0,
                     state->x(), state->y() - 1, Rational(8, 10));

        addSuccessor(state, successors, state->x(), 0,
                     state->x() - 1, state->y(), Rational(1, 10));

        addSuccessor(state, successors, width_ - 1, state->x(),
                     state->x() + 1, state->y(), Rational(1, 10));
    } else if (action->dir() == gridworld::LEFT) {
        addSuccessor(state, successors, state->x(), 0,
                     state->x() - 1, state->y(), Rational(8, 10));

        addSuccessor(state, successors, state->y(), 0,
                     state->x(), state->y() - 1, Rational(1, 10));

        addSuccessor(state, successors, height_ - 1, state->y(),
                     state->x(), state->y() + 1, Rational(1, 10));
    } else if (action->dir() == gridworld::RIGHT) {
        addSuccessor(state, successors, width_ - 1, state->x(),
                     state->x() + 1, state->y(), Rational(8, 10));

        addSuccessor(state, successors, state->y(), 0,
                     state->x(), state->y() - 1, Rational(1, 10));

        addSuccessor(state, successors, height_ - 1, state->y(),
                     state->x(), state->y() + 1, Rational(1, 10));
    }

    return successors;
}

Rational GridWorldProblem::cost(State* s, Action* a) const
{
    return Rational(1);
}

bool GridWorldProblem::applicable(State* s, Action* a) const
{
    return true;
}

const State* GridWorldProblem::getInitialState() const
{
    return s0;
}

void GridWorldProblem::addSuccessor(GridWorldState* state, std::list<Successor>& successors,
                                    int val, int limit, int newx, int newy, Rational prob)
{
    if (val > limit) {
        GridWorldState *next = new GridWorldState(newx, newy);
        successors.push_front(Successor(this->getState(next), prob));
    } else {
        successors.push_front(Successor(state, prob));
    }
}

/*
 * Warning: This method changes the value of the 'visited' variable for all states in
 * the state set . Do not call this method if other code is using these values.
 */
void GridWorldProblem::generateAll()
{
    for (State* state: states)
        state->unvisit();
    std::list<State *> queue;
    queue.push_front(s0);
    while (!queue.empty()) {
        State* cur = queue.front();
        queue.pop_front();
        if (cur->visited())
            continue;
        cur->visit();
        for (Action* a : actions) {
            std::list<Successor> successors = transition(cur, a);
            for (Successor sccr : successors) {
                queue.push_front(sccr.first);
            }
        }
    }
}
