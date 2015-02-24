#include "../../../include/lexi/domains/LexiGridWorldState.h"
#include "../../../include/lexi/domains/LexiGridWorldProblem.h"

#include <cassert>

namespace mllexi
{

LexiGridWorldProblem::LexiGridWorldProblem(int width, int height, int x0, int y0,
                                           std::vector<PairDoubleMap>& goals, int size,
                                           double actionCost)
                                           : width_(width), height_(height), x0_(x0), y0_(y0),
                                             goals_(goals), actionCost_(actionCost)
{
    size_ = size;
    mlcore::State* init = new LexiGridWorldState(this, -2, -2);
    absorbing = new LexiGridWorldState(this, -1, -1);
    s0 = this->addState(init);
    addAllActions();
}

double LexiGridWorldProblem::cost(mlcore::State* s, mlcore::Action* a, int i) const
{
    assert(i < size_);

    if (s == absorbing)
        return 0.0;

    LexiGridWorldState* gws = (LexiGridWorldState *) s;
    GridWorldAction* gwa = (GridWorldAction *) a;
    std::pair<int,int> pos(gws->x(), gws->y());
    std::vector<double> costs;
    for (int i = 0; i < size_; i++) {
        if (goal(s, 0)) {
            PairDoubleMap pdm = goals_[i];
            costs.push_back(pdm[pos]);
        }
        else {
            /* Assigning a higher cost of going down for all value functions except the first */
            if (i > 0 && gwa->dir() == gridworld::DOWN)
                costs.push_back(COST_DOWN_2 * actionCost_);
            else
                costs.push_back(actionCost_);
        }
    }
    return costs[i];
}

bool LexiGridWorldProblem::goal(mlcore::State* s, int index) const
{
    LexiGridWorldState* gws = (LexiGridWorldState *) s;
    std::pair<int,int> pos(gws->x(),gws->y());
    return goals_[index].find(pos) != goals_[index].end();
}

std::list<mlcore::Successor>
LexiGridWorldProblem::transition(mlcore::State* s, mlcore::Action* a, int index)
{

    LexiGridWorldState* state = (LexiGridWorldState *) s;
    GridWorldAction* action = (GridWorldAction *) a;

    std::list<mlcore::Successor> successors;

    if (s == s0) {
        addSuccessor(state, successors, -1, -2, x0_, y0_, 1.0   );
        return successors;
    }

    if (s == absorbing) {
        successors.push_front(mlcore::Successor(s, 1.0));
        return successors;
    }

    if (goal(s, index)) {
        successors.push_front(mlcore::Successor(absorbing, 1.0));
        return successors;
    }

    if (action->dir() == gridworld::UP) {
        addSuccessor(state, successors, height_ - 1, state->y(),
                     state->x(), state->y() + 1, 0.8);

        addSuccessor(state, successors, state->x(), 0,
                     state->x() - 1, state->y(), 0.1);

        addSuccessor(state, successors, width_ - 1, state->x(),
                     state->x() + 1, state->y(), 0.1);
    } else if (action->dir() == gridworld::DOWN) {
        addSuccessor(state, successors, state->y(), 0,
                     state->x(), state->y() - 1, 0.8);

        addSuccessor(state, successors, state->x(), 0,
                     state->x() - 1, state->y(), 0.1);

        addSuccessor(state, successors, width_ - 1, state->x(),
                     state->x() + 1, state->y(), 0.1);
    } else if (action->dir() == gridworld::LEFT) {
        addSuccessor(state, successors, state->x(), 0,
                     state->x() - 1, state->y(), 0.8);

        addSuccessor(state, successors, state->y(), 0,
                     state->x(), state->y() - 1, 0.1);

        addSuccessor(state, successors, height_ - 1, state->y(),
                     state->x(), state->y() + 1, 0.1);
    } else if (action->dir() == gridworld::RIGHT) {
        addSuccessor(state, successors, width_ - 1, state->x(),
                     state->x() + 1, state->y(), 0.8);

        addSuccessor(state, successors, state->y(), 0,
                     state->x(), state->y() - 1, 0.1);

        addSuccessor(state, successors, height_ - 1, state->y(),
                     state->x(), state->y() + 1, 0.1);
    }
    return successors;
}

bool LexiGridWorldProblem::applicable(mlcore::State* s, mlcore::Action* a) const
{
    return true;
}


void LexiGridWorldProblem::addSuccessor(LexiGridWorldState* state, std::list<mlcore::Successor>& successors,
                                    int val, int limit, int newx, int newy, double prob)
{
    if (val > limit) {
        LexiGridWorldState *next = new LexiGridWorldState(this, newx, newy);
        successors.push_front(mlcore::Successor(this->addState(next), prob));
    } else {
        successors.push_front(mlcore::Successor(state, prob));
    }
}

void LexiGridWorldProblem::addAllActions()
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

}
