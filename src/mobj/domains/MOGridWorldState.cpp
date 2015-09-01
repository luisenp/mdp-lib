#include "../../../include/mobj/domains/MOGridWorldState.h"

namespace mlmobj
{

MOGridWorldState::MOGridWorldState(mlcore::Problem* problem, int x, int y) : x_(x), y_(y)
{
    problem_ = problem;
    mobjCost_ = std::vector<double> (((MOProblem *) problem_)->size());
    MOProblem* aux = (MOProblem *) problem_;
    for (int i = 0; i < aux->size(); i++) {
        if (!aux->heuristics().empty())
            mobjCost_[i] = aux->heuristics()[i]->cost(this);
        else
            mobjCost_[i] = 0.0;
    }
}

std::ostream& MOGridWorldState::print(std::ostream& os) const
{
    os << "(" << x_ << "," << y_ << ")";
    return os;
}

bool MOGridWorldState::equals(mlcore::State* other) const
{
    MOGridWorldState* gws = (MOGridWorldState*) other;
    return *this ==  *gws;
}

int MOGridWorldState::hashValue() const
{
    return x_ + 31*y_;
}

int MOGridWorldState::x() const
{
    return x_;
}

int MOGridWorldState::y() const
{
    return y_;
}

}
