#include "../../../include/lexi/domains/LexiGridWorldState.h"

namespace mllexi
{

LexiGridWorldState::LexiGridWorldState(mlcore::Problem* problem, int x, int y) : x_(x), y_(y)
{
    problem_ = problem;
    lexiCost_ = std::vector<double> (((LexiProblem *) problem_)->size());
    LexiProblem* aux = (LexiProblem *) problem_;
    for (int i = 0; i < aux->size(); i++) {
        if (!aux->heuristics().empty())
            lexiCost_[i] = aux->heuristics()[i]->cost(this);
        else
            lexiCost_[i] = 0.0;
    }
}

std::ostream& LexiGridWorldState::print(std::ostream& os) const
{
    os << "(" << x_ << "," << y_ << ")";
    return os;
}

bool LexiGridWorldState::equals(mlcore::State* other) const
{
    LexiGridWorldState* gws = (LexiGridWorldState*) other;
    return *this ==  *gws;
}

int LexiGridWorldState::hashValue() const
{
    return x_ + 31*y_;
}

int LexiGridWorldState::x() const
{
    return x_;
}

int LexiGridWorldState::y() const
{
    return y_;
}

}
