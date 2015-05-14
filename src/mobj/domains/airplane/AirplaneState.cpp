#include "../../../../include/mobj/domains/airplane/AirplaneState.h"

namespace mlmobj
{


AirplaneState::AirplaneState(AirplaneState* state)
{
    airplaneLocation_ = state->airplaneLocation_;
    personLocations_ = state->personLocations_;
    windConditions_ = state->windConditions_;
    problem_ = state->problem_;

    mobjCost_ = std::vector<double> (2);
    MOProblem* aux = (MOProblem*) problem_;
    for (int i = 0; i < 2; i++) {
        if (aux->heuristics().size() > i && aux->heuristics()[i] != nullptr)
            mobjCost_[i] = aux->heuristics()[i]->cost(this);
        else {
            dprint1(this);
            dprint4("HEURI ", i, " ", aux->heuristics().size());
            if (aux->heuristics().size() > i)
                dprint1((void *) aux->heuristics()[i]);
            dsleep(50);
            mobjCost_[i] = 0.0;
        }
    }
}


AirplaneState::AirplaneState(int airplaneLocation,
                             std::vector<int> personLocations,
                             std::vector<int> windConditions,
                             MOProblem* problem)
{
    airplaneLocation_ = airplaneLocation;
    personLocations_ = personLocations;
    windConditions_ = windConditions;
    problem_ = problem;

    mobjCost_ = std::vector<double> (2);
    for (int i = 0; i < 2; i++) {
        if (problem->heuristics().size() > i && problem->heuristics()[i] != nullptr)
            mobjCost_[i] = problem->heuristics()[i]->cost(this);
        else
            mobjCost_[i] = 0.0;
    }
}


std::ostream& AirplaneState::print(std::ostream& os) const
{
    AirplaneState* rtp = (AirplaneState*) problem_;
    os << "airplane: " << airplaneLocation_  << ", persons:";

    for (int i = 0; i < personLocations_.size(); i++)
        os << " " << personLocations_[i] << ",";

    os << " wind: ";
    for (int i = 0; i < windConditions_.size(); i++) {
        os << windConditions_[i];
        if (i < windConditions_.size() - 1)
            os << ", ";
    }
    return os;
}


mlcore::State& AirplaneState::operator=(const mlcore::State& rhs)
{
    if (this == &rhs)
        return *this;

    AirplaneState* state = (AirplaneState*)  & rhs;
    airplaneLocation_ = state->airplaneLocation_;
    personLocations_ = state->personLocations_;
    windConditions_ = state->windConditions_;
    return *this;
}

bool AirplaneState::operator==(const mlcore::State& rhs) const
{
    AirplaneState* state = (AirplaneState*)  & rhs;

    if (personLocations_.size() != state->personLocations_.size())
        return false;
    for (int i = 0; i < personLocations_.size(); i++)
        if (personLocations_[i] != state->personLocations_[i])
            return false;

    if (windConditions_.size() != state->windConditions_.size())
        return false;
    for (int i = 0; i < windConditions_.size(); i++)
        if (windConditions_[i] != state->windConditions_[i])
            return false;

    return airplaneLocation_ == state->airplaneLocation_;
}

bool AirplaneState::equals(mlcore::State* other) const
{
    AirplaneState* state = (AirplaneState*) other;
    return *this ==  *state;
}

int AirplaneState::hashValue() const
{
    int hash = 0;

    for (int i = 0; i < personLocations_.size(); i++)
        hash = personLocations_[i] + 31*hash;

    for (int i = 0; i < windConditions_.size(); i++)
        hash = windConditions_[i] + 31*hash;

    hash = airplaneLocation_ + 31*hash;
}

}
