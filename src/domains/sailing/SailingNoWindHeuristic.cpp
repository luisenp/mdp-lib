#include <algorithm>
#include <cmath>

#include "../../../include/domains/sailing/SailingAction.h"
#include "../../../include/domains/sailing/SailingNoWindHeuristic.h"
#include "../../../include/domains/sailing/SailingProblem.h"
#include "../../../include/domains/sailing/SailingState.h"

#include "../../../include/Action.h"
#include "../../../include/Problem.h"
#include "../../../include/State.h"

SailingNoWindHeuristic::SailingNoWindHeuristic(SailingProblem* problem)
{
    problem_ = problem;
    tmp_ = new SailingState(-2, -2, -2, problem_);
}

double SailingNoWindHeuristic::cost(const mlcore::State* s)
{
    const SailingState* state = static_cast<const SailingState*> (s);
    if (state->x() < 0
        || (state->x() == problem_->goalX_ &&
                state->y() == problem_->goalY_))   // absorbing state
        return 0.0;

    int x = state->x_;
    int y = state->y_;
    tmp_->x_ = x;
    tmp_->y_ = y;
    tmp_->wind_ = state->wind_;
    double heuristicValue = mdplib::dead_end_cost + 1;
    for (mlcore::Action* a : problem_->actions()) {
        SailingAction* sa = static_cast<SailingAction*> (a);
        double cost = problem_->cost(tmp_, a);
        short dx[] = {0, 1, 1,  1,  0, -1, -1, -1};
        short dy[] = {1, 1, 0, -1, -1, -1,  0,  1};
        short nextX = (short) (x + dx[sa->dir()]);
        short nextY = (short) (y + dy[sa->dir()]);
        int deltaX = std::fabs(nextX - problem_->goalX_);
        int deltaY = std::fabs(nextY - problem_->goalY_);
        heuristicValue = std::min(heuristicValue, std::max(deltaX, deltaY) + cost);
    }
    return heuristicValue;
}
