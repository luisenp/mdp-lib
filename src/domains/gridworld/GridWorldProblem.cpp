#include "../../../include/problem.h"
#include "../../../include/domains/gridworld/GridWorldProblem.h"
#include "../../../include/domains/gridworld/GridWorldState.h"
#include "../../../include/domains/gridworld/GridWorldAction.h"
#include "../../../include/util/rational.h"

void GridWorldProblem::addAllActions()
{
    mlcore::Action* up = new GridWorldAction(gridworld::UP);
    mlcore::Action* down = new GridWorldAction(gridworld::DOWN);
    mlcore::Action* left = new GridWorldAction(gridworld::LEFT);
    mlcore::Action* right = new GridWorldAction(gridworld::RIGHT);
    actions_.push_front(up);
    actions_.push_front(down);
    actions_.push_front(left);
    actions_.push_front(right);
}

GridWorldProblem::GridWorldProblem() :
                    width_(0), height_(0), x0_(0), y0_(0), goals_(0)
{
    absorbing = new GridWorldState(this, -1, -1);
    gamma_ = Rational(1);
    addAllActions();
}

GridWorldProblem::GridWorldProblem(int width, int height, int x0, int y0, PairRationalMap* goals)
                                   : width_(width), height_(height), x0_(x0), y0_(y0), goals_(goals)
{
    mlcore::State* init = new GridWorldState(this, x0_, y0_);
    absorbing = new GridWorldState(this, -1, -1);
    s0 = this->addState(init);
    gamma_ = Rational(1);
    addAllActions();
}

GridWorldProblem::GridWorldProblem(int width, int height,
                                   int x0, int y0,
                                   PairRationalMap* goals, mlcore::Heuristic* h)
                                   : width_(width), height_(height),
                                      x0_(x0), y0_(y0), goals_(goals)
{
    mlcore::State* init = new GridWorldState(this, x0_, y0_);
    absorbing = new GridWorldState(this, -1, -1);
    s0 = this->addState(init);
    heuristic_ = h;
    gamma_ = Rational(1);
    addAllActions();
}

bool GridWorldProblem::goal(mlcore::State* s) const
{
    GridWorldState* gws = (GridWorldState *) s;
    std::pair<int,int> pos(gws->x(),gws->y());
    return goals_->find(pos) != goals_->end();
}

std::list<mlcore::Successor> GridWorldProblem::transition(mlcore::State *s, mlcore::Action *a)
{
    GridWorldState* state = (GridWorldState *) s;
    GridWorldAction* action = (GridWorldAction *) a;

    std::list<mlcore::Successor> successors;

    if (s == absorbing) {
        successors.push_front(mlcore::Successor(s, Rational(1)));
        return successors;
    }

    if (goal(s)) {
        successors.push_front(mlcore::Successor(absorbing, Rational(1)));
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

Rational GridWorldProblem::cost(mlcore::State* s, mlcore::Action* a) const
{
    if (s == absorbing)
        return Rational(0);
    if (goal(s)) {
        GridWorldState* gws = (GridWorldState *) s;
        std::pair<int,int> pos(gws->x(),gws->y());
        return (*goals_)[pos];
    }
    return Rational(3, 100);
}

bool GridWorldProblem::applicable(mlcore::State* s, mlcore::Action* a) const
{
    return true;
}

void GridWorldProblem::addSuccessor(GridWorldState* state, std::list<mlcore::Successor>& successors,
                                    int val, int limit, int newx, int newy, Rational prob)
{
    if (val > limit) {
        GridWorldState *next = new GridWorldState(this, newx, newy);
        successors.push_front(mlcore::Successor(this->addState(next), prob));
    } else {
        successors.push_front(mlcore::Successor(state, prob));
    }
}
