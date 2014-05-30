#include "../../../include/domains/ctp/CTPState.h"
#include "../../../include/domains/ctp/CTPProblem.h"

CTPState::CTPState(CTPProblem* problem)
{
    problem_ = problem;
    location_ = 0;
    initAllUnkown();
}

CTPState::CTPState(CTPProblem* problem, int location)
{
    problem_ = problem;
    location_ = location;
    initAllUnkown();
}

CTPState::CTPState(CTPState& rhs)
{
    problem_ = rhs.problem_;
    location_ = rhs.location_;
    status_ = rhs.status_;
}


std::ostream& CTPState::print(std::ostream& os) const
{
    return os;
}

bool CTPState::equals(State* other) const
{
    return false;
}

int CTPState::hashValue() const
{
    return 0;
}


void CTPState::initAllUnkown()
{
    CTPProblem* pr = (CTPProblem *) problem_;
    for (int i = 0; i < pr->roads().numVertices(); i++) {
        status_.push_back(std::vector<unsigned char> (pr->roads().numVertices()));
        for (int j = 0; j < pr->roads().numVertices(); j++) {
            status_[i][j] = ctp::unknown;
        }
    }

}
