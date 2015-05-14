#include "../../../include/mobj/domains/MORacetrackProblem.h"
#include "../../../include/mobj/domains/MORacetrackState.h"

#include "../../../include/domains/racetrack/RacetrackProblem.h"
#include "../../../include/domains/racetrack/RacetrackState.h"
#include "../../../include/domains/racetrack/RacetrackAction.h"


namespace mlmobj
{

MORacetrackState::MORacetrackState(int x, int y,
                                       int vx, int vy,
                                       bool safe, MOProblem* problem)
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

    std::vector<double> weights(problem->size(), 0.0);
    resetCost(weights, -1);
}


std::ostream& MORacetrackState::print(std::ostream& os) const
{
    MORacetrackProblem* rtp = (MORacetrackProblem*) problem_;
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


mlcore::State& MORacetrackState::operator=(const mlcore::State& rhs)
{
    if (this == &rhs)
        return *this;

    MORacetrackState* state = (MORacetrackState*)  & rhs;
    x_ = state->x_;
    y_ = state->y_;
    vx_ = state->vx_;
    vy_ = state->vy_;
    safe_ = state->safe_;
    problem_ = state->problem_;
    return *this;
}


bool MORacetrackState::operator==(const mlcore::State& rhs) const
{
    MORacetrackState* state = (MORacetrackState*)  & rhs;
    return x_ == state->x_
            && y_ == state->y_
            && vx_ == state->vx_
            && vy_ == state->vy_
            && safe_ == state->safe_;
}


bool MORacetrackState::equals(mlcore::State* other) const
{
    MORacetrackState* state = (MORacetrackState*) other;
    return *this ==  *state;
}


int MORacetrackState::hashValue() const
{
    return x_ + 31*(y_ + 31*(vx_ + 31*(vy_ + 31*safe_)));
}

}
