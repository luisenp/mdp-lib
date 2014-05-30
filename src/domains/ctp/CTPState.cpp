#include "../../../include/domains/ctp/CTPState.h"
#include "../../../include/domains/ctp/CTPProblem.h"

CTPState::CTPState(CTPProblem* problem)
{
    problem_ = problem;
    location_ = 0;
    initAllUnkown();
    frontier_.insert(location_);
}

CTPState::CTPState(CTPProblem* problem, int location)
{
    problem_ = problem;
    location_ = location;
    initAllUnkown();
    frontier_.insert(location_);
}

CTPState::CTPState(CTPState& rhs)
{
    problem_ = rhs.problem_;
    location_ = rhs.location_;
    status_ = rhs.status_;
    frontier_ = rhs.frontier_;
}

std::ostream& CTPState::print(std::ostream& os) const
{
    os << "LOCATION: " << location_;
    os << " OPEN: ";
    CTPProblem* ctpp = (CTPProblem* ) problem_;
    int n = ctpp->roads().numVertices();
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (status_[i][j] == ctp::open)
                os << "(" << i << "," << j << ") ";
        }
    }

    os << "BLOCKED: ";
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (status_[i][j] == ctp::blocked)
                os << "(" << i << "," << j << ") ";
        }
    }
    return os;
}

bool CTPState::equals(State* other) const
{
    CTPState* ctps = (CTPState*) other;
    return location_ == ctps->location_ && status_ == ctps->status_;
}

int CTPState::hashValue() const
{
    int hash = location_;
    CTPProblem* ctpp = (CTPProblem* ) problem_;
    int n = ctpp->roads().numVertices();
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            hash = 31*hash + status_[i][j];
        }
    }
    return hash;
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

void CTPState::setStatus(int i, int j, unsigned char st)
{
    assert(i < status_.size() && j < status_.size());
    assert(st == ctp::blocked || st == ctp::open || st == ctp::unknown);
    status_[i][j] = st;
}
