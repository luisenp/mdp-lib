#include "../../../include/mobj/domains/MOGridWorldState.h"
#include "../../../include/mobj/domains/MOGridWorldProblem.h"

#include <cassert>

namespace mlmobj
{

MOGridWorldProblem::MOGridWorldProblem(int width, int height, int x0, int y0,
                                           std::vector<PairDoubleMap>& goals, int size,
                                           double actionCost)
                                           : width_(width), height_(height), x0_(x0), y0_(y0),
                                             goals_(goals), actionCost_(actionCost)
{
    size_ = size;
    mlcore::State* init = new MOGridWorldState(this, -2, -2);
    absorbing = new MOGridWorldState(this, -1, -1);
    s0 = this->addState(init);
    absorbing = this->addState(absorbing);
    addAllActions();
}

double MOGridWorldProblem::cost(mlcore::State* s, mlcore::Action* a, int i) const
{
    assert(i < size_);

    if (s == absorbing || s == s0)
        return 0.0;

    MOGridWorldState* gws = (MOGridWorldState *) s;
    GridWorldAction* gwa = (GridWorldAction *) a;
    std::pair<int,int> pos(gws->x(), gws->y());
    std::vector<double> costs;
    for (int i = 0; i < size_; i++) {
        if (goal(s, 0)) {
            PairDoubleMap pdm = goals_[0];
            costs.push_back(pdm[pos]);
        }
        else {
            costs.push_back( (1.0 + 0.1*((gwa->dir() + i) % 4)) + actionCost_);

            // Assigning a higher cost to DOWN, for all value functions except the first
//            if (i > 0 && gwa->dir() == gridworld::DOWN)
//                costs.push_back(COST_DOWN_2 * actionCost_);
//            else
//                costs.push_back(actionCost_);
        }
    }
    return costs[i];
}

bool MOGridWorldProblem::goal(mlcore::State* s, int index) const
{
    MOGridWorldState* gws = (MOGridWorldState *) s;
    std::pair<int,int> pos(gws->x(),gws->y());
    return goals_[index].find(pos) != goals_[index].end();
}

std::list<mlcore::Successor>
MOGridWorldProblem::transition(mlcore::State* s, mlcore::Action* a, int index)
{

    MOGridWorldState* state = (MOGridWorldState *) s;
    GridWorldAction* action = (GridWorldAction *) a;

    std::list<mlcore::Successor> successors;

    if (s == s0) {
        addSuccessor(state, successors, -1, -2, x0_, y0_, 1.0);
        return successors;
    }

    if (s == absorbing) {
        successors.push_front(mlcore::Successor(s, 1.0));
//        successors.push_front(mlcore::Successor(s0, 1.0));
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

bool MOGridWorldProblem::applicable(mlcore::State* s, mlcore::Action* a) const
{
    return true;
}


void MOGridWorldProblem::addSuccessor(MOGridWorldState* state, std::list<mlcore::Successor>& successors,
                                    int val, int limit, int newx, int newy, double prob)
{
    if (val > limit) {
        MOGridWorldState *next = new MOGridWorldState(this, newx, newy);
        successors.push_front(mlcore::Successor(this->addState(next), prob));
    } else {
        successors.push_front(mlcore::Successor(state, prob));
    }
}

void MOGridWorldProblem::addAllActions()
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
