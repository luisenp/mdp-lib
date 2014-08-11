#include <algorithm>

#include "../../../include/domains/sailing/SailingProblem.h"
#include "../../../include/domains/sailing/SailingState.h"
#include "../../../include/domains/sailing/SailingAction.h"

SailingProblem::SailingProblem(short startX, short startY,
                               short goalX, short goalY,
                               short rows, short cols,
                               std::vector<double> & costs)
{
    startX_ = startX;
    startY_ = startY;
    goalX_ = goalX;
    goalY_ = goalY;
    rows_ = rows;
    cols_ = cols;
    costs_ = costs;

    mlcore::State* absorbing_ = new SailingState(-1, -1, -1);
    this->addState(absorbing_);
}


bool SailingProblem::inLake(SailingState* state)
{
    short x = state->x();
    short y = state->y();
    return x >= 0 && x < rows_ && y >= 0 && y <= cols_;
}

int SailingProblem::tack(SailingState* state, SailingAction* action) const
{
    short d = abs(action->dir() - state->wind());
    return std::min((int) d, 8 - d);
}

bool SailingProblem::goal(mlcore::State* s) const
{
    SailingState* state = (SailingState *) s;
    return state->x() == goalX_ && state->y() == goalY_;
}

std::list<mlcore::Successor> SailingProblem::transition(mlcore::State* s, mlcore::Action* a)
{
    std::list<mlcore::Successor> successors;

    if (goal(s) || s == absorbing_) {
        successors.push_back(mlcore::Successor(absorbing_, 1.0));
        return successors;
    }

    SailingState* state = (SailingState *) s;
    SailingAction* action = (SailingAction*) a;

    if (tack(state, action) != INTO) {
        short dx[] = {0, 1, 1,  1,  0, -1, -1, -1};
        short dy[] = {1, 1, 0, -1, -1, -1,  0,  1};
        short nextX = (short) (state->x() + dx[action->dir()]);
        short nextY = (short) (state->y() + dy[action->dir()]);
    }

    return successors;
}

double SailingProblem::cost(mlcore::State* s, mlcore::Action* a) const
{
    return goal(s) ? 0.0 : costs_[tack((SailingState *) s, (SailingAction *) a)];
}

bool SailingProblem::applicable(mlcore::State* s, mlcore::Action* a) const
{
    return true;
}
