#include "../../../include/domains/racetrack/RacetrackProblem.h"
#include "../../../include/domains/racetrack/RacetrackState.h"
#include "../../../include/domains/racetrack/RacetrackAction.h"


RacetrackState::RacetrackState(int x, int y, int vx, int vy,
                                mlcore::Problem* problem)
{
    x_ = x;
    y_ = y;
    vx_ = vx;
    vy_ = vy;
    problem_ = problem;

    /* Adding a successor entry for each action */
    for (int i = 0; i < 9; i++) {
        allSuccessors_.push_back(std::list<mlcore::Successor> ());
    }
}

std::ostream& RacetrackState::print(std::ostream& os) const
{
    RacetrackProblem* rtp = static_cast<RacetrackProblem *> (problem_);
    os << "(" << x_  << ", " << y_ << ", " << vx_ << ", " << vy_ << ", w";
    if (x_ >= 0) {
        if  (rtp->track()[x_][y_] == rtrack::wall)
            os << "1";
        else
            os << "0";
    }
    os << ")";
    return os;
}

mlcore::State& RacetrackState::operator=(const mlcore::State& rhs)
{
    if (this == &rhs)
        return *this;

    const RacetrackState* state = static_cast<const RacetrackState *> (&rhs);
    x_ = state->x_;
    y_ = state->y_;
    vx_ = state->vx_;
    vy_ = state->vy_;
    problem_ = state->problem_;
    return *this;
}

bool RacetrackState::operator==(const mlcore::State& rhs) const
{
    const RacetrackState* state = static_cast<const RacetrackState *> (&rhs);
    return x_ == state->x_
            && y_ == state->y_
            && vx_ == state->vx_
            && vy_ == state->vy_;
}
bool RacetrackState::equals(mlcore::State* other) const
{
    RacetrackState* state = static_cast<RacetrackState *> (other);;
    return *this ==  *state;
}

int RacetrackState::hashValue() const
{
    return x_ + 31 * (y_ + 31 * (vx_ + 31 * vy_));
}
