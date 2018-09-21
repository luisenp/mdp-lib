#include <algorithm>

#include "../../../include/domains/sailing/SailingAction.h"
#include "../../../include/domains/sailing/SailingProblem.h"
#include "../../../include/domains/sailing/SailingState.h"

SailingProblem::SailingProblem(short startX, short startY,
                               short startWind,
                               short goalX, short goalY,
                               short rows, short cols,
                               std::vector<double> & costs,
                               double* windTransition,
                               bool useFlatTransition)
{
    startX_ = startX;
    startY_ = startY;
    goalX_ = goalX;
    goalY_ = goalY;
    rows_ = rows;
    cols_ = cols;
    costs_ = costs;
    windTransition_ = windTransition;
    useFlatTransition_ = useFlatTransition;

    absorbing_ = new SailingState(-1, -1, -1, this);
    s0 = new SailingState(startX, startY, startWind, this);
    this->addState(s0);
    this->addState(absorbing_);

    for (short i = 0; i < 8; i++) {
        mlcore::Action* a = new SailingAction(i);
        actions_.push_back(a);
    }
}


bool SailingProblem::inLake(const short x, const short y) const
{
    return x >= 0 && x < rows_ && y >= 0 && y < cols_;
}


int SailingProblem::tack(const SailingState* state,
                          const SailingAction* action) const
{
    short d = abs(action->dir() - state->wind());
    return std::min((int) d, 8 - d);
}


bool SailingProblem::goal(mlcore::State* s) const
{
    if (s == absorbing_)
      return true;
    SailingState* state = static_cast<SailingState*> (s);
    return state->x() == goalX_ && state->y() == goalY_;
}


std::list<mlcore::Successor>
SailingProblem::transition(mlcore::State* s, mlcore::Action* a)
{
    std::list<mlcore::Successor> successors;

    if (goal(s) || s == absorbing_) {
        if (useFlatTransition_)
            for (int i = 0; i < 8; i++)
                successors.push_back(mlcore::Successor(absorbing_, 0.125));
        else {
            successors.push_back(mlcore::Successor(absorbing_, 1.0));
        }
        return successors;
    }

    SailingState* state = static_cast<SailingState*> (s);
    SailingAction* action = static_cast<SailingAction*> (a);

    if (tack(state, action) != INTO) {
        short dx[] = {0, 1, 1,  1,  0, -1, -1, -1};
        short dy[] = {1, 1, 0, -1, -1, -1,  0,  1};
        short nextX = (short) (state->x() + dx[action->dir()]);
        short nextY = (short) (state->y() + dy[action->dir()]);

        if (inLake(nextX, nextY)) {
            for (short nextWind = 0; nextWind < 8; nextWind++) {
                double p = windTransition_[8 * state->wind() + nextWind];
                if (p > 0.0 || useFlatTransition_) {
                    mlcore::State* next =
                        new SailingState(nextX, nextY, nextWind, this);
                    successors.push_back(
                        mlcore::Successor(this->addState(next), p));
                }
            }
        } else {
            if (useFlatTransition_) {
                for (int i = 0; i < 8; i++)
                    successors.push_back(mlcore::Successor(state, 0.125));
            } else {
                successors.push_back(mlcore::Successor(state, 1.0));
            }
        }
    } else {
        if (useFlatTransition_) {
            for (int i = 0; i < 8; i++)
                successors.push_back(mlcore::Successor(state, 0.125));
        } else {
            successors.push_back(mlcore::Successor(state, 1.0));
        }
    }
    return successors;
}


double SailingProblem::cost(mlcore::State* s, mlcore::Action* a) const
{
    return goal(s) ? 0.0 : costs_[tack(static_cast<SailingState*> (s),
                                        static_cast<SailingAction*> (a))];
}


bool SailingProblem::applicable(mlcore::State* s, mlcore::Action* a) const
{
    SailingState* state = static_cast<SailingState*> (s);
    SailingAction* action = static_cast<SailingAction*> (a);
    if (tack(state, action) != INTO) {
        short dx[] = {0, 1, 1,  1,  0, -1, -1, -1};
        short dy[] = {1, 1, 0, -1, -1, -1,  0,  1};
        short nextX = (short) (state->x() + dx[action->dir()]);
        short nextY = (short) (state->y() + dy[action->dir()]);

        return inLake(nextX, nextY);
    }
    return true;
}
