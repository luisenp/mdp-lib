#include <cassert>

#include "../../../include/State.h"

#include "../../../../include/mobj/domains/airplane/AirplaneHeuristic.h"
#include "../../../../include/mobj/domains/airplane/AirplaneProblem.h"
#include "../../../../include/mobj/domains/airplane/AirplaneState.h"

namespace mlmobj
{

double AirplaneHeuristic::cost(const mlcore::State* s) const
{

    AirplaneState* state = (AirplaneState*) s;

    if (state == problem_->s0 ||
        problem_->goal(state) ||
        state == problem_->absorbing_)
        return 0.0;

    int loadsNeeded = problem_->travelGoal_;
    for (int i = 0; i < state->personLocations_.size(); i++)
        loadsNeeded -= (state->personLocations_[i] == INSIDE_AIRPLANE
                        || state->personLocations_[i] == 0);

    int unloadsNeeded = problem_->travelGoal_;
    for (int i = 0; i < state->personLocations_.size(); i++)
        unloadsNeeded -= (state->personLocations_[i] == 0);

    int tripsNeeded = problem_->travelGoal_ + 1;
    int personsLoaded = 0;
    for (int i = 0; i < state->personLocations_.size(); i++) {
        personsLoaded +=
            (state->personLocations_[i] == 0
                || state->personLocations_[i] == INSIDE_AIRPLANE
                || state->personLocations_[i] == state->airplaneLocation_);
    }
    personsLoaded = std::min(personsLoaded, problem_->travelGoal_);
    tripsNeeded -= personsLoaded;
    if (tripsNeeded == 1 && state->airplaneLocation_ == 0)
        tripsNeeded = 0;

    if (level_ == 0)
        return problem_->minDistance_ * tripsNeeded +
                loadsNeeded + unloadsNeeded;

    double costFirstTrip = 0.0;
    if (tripsNeeded > 0) {
        if (state->windConditions_[state->airplaneLocation_] ==
                AIRPLANE_WINDY) {
            costFirstTrip *= 2;
            tripsNeeded--;
        }
    }
    return problem_->minDistance_ * tripsNeeded +
            loadsNeeded + unloadsNeeded + costFirstTrip;
}

}
