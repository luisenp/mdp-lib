#include <vector>
#include <cassert>

#include "../../../../include/mobj/domains/airplane/AirplaneProblem.h"
#include "../../../../include/mobj/domains/airplane/AirplaneState.h"
#include "../../../../include/mobj/domains/airplane/AirplaneAction.h"

namespace mlmobj
{

AirplaneProblem::AirplaneProblem(int travelGoal,
                                 std::vector< std::vector<double> > distances,
                                 std::vector< double > windProbs,
                                 std::vector< int > initialLocations)
{
    size_ = 2;

    travelGoal_ = travelGoal;
    distances_ = distances;
    windProbs_ = windProbs;
    initialLocations_ = initialLocations;

    s0 = new AirplaneState(-1,  std::vector<int> (),  std::vector<int> (), this);
    this->addState(s0);
    absorbing_ = new AirplaneState(-2,  std::vector<int> (),  std::vector<int> (), this);
    this->addState(absorbing_);

    int npersons = initialLocations_.size();
    for (int i = 0; i < npersons; i++) {
        actions_.push_back(new AirplaneAction(AIRPLANE_LOAD, i, -1));
        actions_.push_back(new AirplaneAction(AIRPLANE_UNLOAD, i, -1));
    }
    int ncities = distances.size();
    for (int i = 0; i < ncities; i++) {
        for (int j = 0; j < ncities; j++) {
            if (i != j) {
                actions_.push_back(new AirplaneAction(AIRPLANE_FLY, i, j));
                minDistance_ = std::min(minDistance_, distances_[i][j]);
            }
        }
    }
}


bool AirplaneProblem::applicable(mlcore::State* s, mlcore::Action* a) const
{
    AirplaneState* state = (AirplaneState *) s;
    AirplaneAction* action = (AirplaneAction *) a;

    if (s == s0 || s == absorbing_)
        return true;

    if (action->type_ == AIRPLANE_LOAD)
        return state->personLocations_[action->par1_] == state->airplaneLocation_;

    else if (action->type_ == AIRPLANE_UNLOAD)
        return state->personLocations_[action->par1_] == INSIDE_AIRPLANE;

    else {
        if (action->type_ != AIRPLANE_FLY)
            return false;

        if (action->par2_  < 0 || action->par2_ >= distances_.size()
            || action->par2_ == INSIDE_AIRPLANE
            || state->airplaneLocation_ != action->par1_)
            return false;

//        if (action->par2_ != 0) {
//            int cnt = 0;
//            for (int i = 0; i < state->personLocations_.size(); i++) {
//                if (state->personLocations_[i] == action->par2_)
//                    cnt++;
//            }
//            return cnt > 0;
//        }
    }

    return true;
}

double AirplaneProblem::cost(mlcore::State* s, mlcore::Action* a, int i) const
{
    AirplaneState* state = (AirplaneState *) s;
    AirplaneAction* action = (AirplaneAction *) a;

    if (s == s0 || goal(s) || s == absorbing_)
        return 0.0;

    if (action->type_ == AIRPLANE_LOAD || action->type_ == AIRPLANE_UNLOAD)
        return 1.0;

    if (i == 0) {
        return distances_[action->par1_][action->par2_];
    } else {
        double mult =
            state->windConditions_[action->par1_] + state->windConditions_[action->par2_] + 1;
        return mult * distances_[action->par1_][action->par2_];
    }

}

mlcore::SuccessorsList AirplaneProblem::transition(mlcore::State* s, mlcore::Action* a, int index)
{
    assert(applicable(s, a));

    mlcore::SuccessorsList successors;

    if (s == s0) {
        int ncities = distances_.size();
        mlcore::State* next =
            this->addState(new AirplaneState(0,
                                         initialLocations_,
                                         std::vector<int> (ncities, AIRPLANE_CALM),
                                         this));
        successors.push_back(mlcore::Successor(next, 1.0));
        return successors;
    }

    if (goal(s) || s == absorbing_) {
        successors.push_back(mlcore::Successor(absorbing_, 1.0));
        return successors;
    }

    AirplaneState* state = (AirplaneState *) s;
    AirplaneAction* action = (AirplaneAction *) a;

    AirplaneState* nextProto = new AirplaneState(state);
    if (action->type_ == AIRPLANE_LOAD)
        nextProto->personLocations_[action->par1_] = INSIDE_AIRPLANE;
    else if (action->type_ == AIRPLANE_UNLOAD)
        nextProto->personLocations_[action->par1_] = state->airplaneLocation_;
    else if (action->type_ == AIRPLANE_FLY)
        nextProto->airplaneLocation_ = action->par2_;

    int n = distances_.size();
    for (int mask = 0; mask < (1<<n); mask++) {
        AirplaneState* next = new AirplaneState(nextProto);
        double prob = 1.0;
        for (int i = 0; i < n; i++) {
            if (mask & (1<<i)) {
                prob *= windProbs_[i];
                next->windConditions_[i] = AIRPLANE_WINDY;
            } else {
                prob *= (1 - windProbs_[i]);
                next->windConditions_[i] = AIRPLANE_CALM;
            }
        }
        successors.push_back(mlcore::Successor(this->addState(next), prob));
    }

    return successors;
}

bool AirplaneProblem::goal(mlcore::State* s, int index) const
{
    AirplaneState * state = (AirplaneState *) s;

    if (s == s0)
        return false;

    int cnt = 0;
    for (int i = 0; i < state->personLocations_.size(); i++)
        if (state->personLocations_[i] == 0)
            cnt++;
    return cnt == travelGoal_;
}

}
