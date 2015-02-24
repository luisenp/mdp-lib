#include "../../../include/lexi/domains/LexiRacetrackProblem.h"
#include "../../../include/lexi/domains/LexiRacetrackState.h"

#include "../../../include/domains/racetrack/RacetrackProblem.h"
#include "../../../include/domains/racetrack/RacetrackState.h"
#include "../../../include/domains/racetrack/RacetrackAction.h"


namespace mllexi
{

LexiRacetrackState::LexiRacetrackState(int x, int y,
                                       int vx, int vy,
                                       bool safe, LexiProblem* problem)
{
    x_ = x;
    y_ = y;
    vx_ = vx;
    vy_ = vy;
    safe_ = safe;
    problem_ = problem;

    /* Adding a successor entry for each action */
    for (int i = 0; i < 9; i++) {
        allSuccessors_.push_back(std::list<mlcore::Successor> ());
    }

    lexiCost_ = std::vector<double> (problem->size());
    for (int i = 0; i < problem->size(); i++) {
        if (problem->heuristics().size() > i && problem->heuristics()[i] != nullptr)
            lexiCost_[i] = problem->heuristics()[i]->cost(this);
        else
            lexiCost_[i] = 0.0;
    }
}

std::ostream& LexiRacetrackState::print(std::ostream& os) const
{
    LexiRacetrackProblem* rtp = (LexiRacetrackProblem*) problem_;
    os << "(" << x_  << ", " << y_ << ", " << vx_ << ", " << vy_ << ", w";
    if (x_ >= 0) {
        if  (rtp->track()[x_][y_] == rtrack::wall)
            os << "1";
        else
            os << "0";
    }
    os << ", s" << safe_ << ")";
    return os;
}

mlcore::State& LexiRacetrackState::operator=(const mlcore::State& rhs)
{
    if (this == &rhs)
        return *this;

    LexiRacetrackState* state = (LexiRacetrackState*)  & rhs;
    x_ = state->x_;
    y_ = state->y_;
    vx_ = state->vx_;
    vy_ = state->vy_;
    safe_ = state->safe_;
    problem_ = state->problem_;
    return *this;
}

bool LexiRacetrackState::operator==(const mlcore::State& rhs) const
{
    LexiRacetrackState* state = (LexiRacetrackState*)  & rhs;
    return x_ == state->x_
            && y_ == state->y_
            && vx_ == state->vx_
            && vy_ == state->vy_
            && safe_ == state->safe_;
}
bool LexiRacetrackState::equals(mlcore::State* other) const
{
    LexiRacetrackState* state = (LexiRacetrackState*) other;
    return *this ==  *state;
}

int LexiRacetrackState::hashValue() const
{
    return x_ + 31*(y_ + 31*(vx_ + 31*(vy_ + 31*safe_)));
}

}
