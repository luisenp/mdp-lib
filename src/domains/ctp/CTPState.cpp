#include "../../../include/domains/ctp/CTPState.h"
#include "../../../include/domains/ctp/CTPProblem.h"

#include "../../../include/util/general.h"

CTPState::CTPState(CTPProblem* problem)
{
    problem_ = problem;
    location_ = 0;
    initAllUnkown();
    badWeather_ = ctp::UNKNOWN;
}

CTPState::CTPState(CTPProblem* problem, int location)
{
    problem_ = problem;
    location_ = location;
    initAllUnkown();
    badWeather_ = ctp::UNKNOWN;
}

CTPState::CTPState(CTPState& rhs)
{
    problem_ = rhs.problem_;
    location_ = rhs.location_;
    status_ = rhs.status_;
    explored_ = rhs.explored_;
    badWeather_ = ctp::UNKNOWN;
}

std::ostream& CTPState::print(std::ostream& os) const
{
    if (location_ == -1) {
        os << "Absorbing State";
        return os;
    }
    os << "Location: " << location_;
    os << " Open: ";
    CTPProblem* ctpp = (CTPProblem* ) problem_;
    int n = ctpp->roads()->numVertices();
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (status_[i][j] == ctp::OPEN)
                os << "(" << i << "," << j << ") ";
        }
    }

    os << "Blocked: ";
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (status_[i][j] == ctp::BLOCKED)
                os << "(" << i << "," << j << ") ";
        }
    }

    os << "Explored: ";
    for (int x : explored_)
        os << x << " ";
    return os;
}

bool CTPState::equals(State* other) const
{
    CTPState* ctps = static_cast<CTPState*>(other);
    return *this ==  *ctps;
}

int CTPState::hashValue() const
{
    int hash = location_;
    CTPProblem* ctpp = static_cast<CTPProblem*>(problem_);
    int n = ctpp->roads()->numVertices();
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            hash = 31*hash + status_[i][j];
        }
    }
    return hash;
}

void CTPState::initAllUnkown()
{
    CTPProblem* pr = static_cast<CTPProblem*>(problem_);
    for (int i = 0; i < pr->roads()->numVertices(); i++) {
        status_.push_back(
            std::vector<unsigned char> (pr->roads()->numVertices()));
        for (int j = 0; j < pr->roads()->numVertices(); j++) {
            status_[i][j] = ctp::UNKNOWN;
        }
    }
}

void CTPState::setStatus(int i, int j, unsigned char st)
{
    assert(i < status_.size() && j < status_.size());
    assert(st == ctp::BLOCKED || st == ctp::OPEN || st == ctp::UNKNOWN);
    status_[i][j] = st;
}

bool CTPState::reachable(int v)
{
    if (location_ == v)
        return true;
    Graph* g = static_cast<CTPProblem*>(problem_)->roads();
    std::list<int> Q;
    Q.push_front(location_);
    std::unordered_set<int> visited;
    while (!Q.empty()) {
        int tmp = Q.front();
        Q.pop_front();
        visited.insert(tmp);
        std::unordered_map<int,double> neighbors = g->neighbors(tmp);
        for (std::pair<int, double> ne : neighbors) {
            int x = ne.first;
            if (status_[tmp][x] != ctp::OPEN)
                continue;
            if (visited.find(x) != visited.end())
                continue;
            if (x == v)
                return true;
            Q.push_front(x);
        }
    }
    return false;
}

bool CTPState::potentiallyReachable(int v)
{
    if (location_ == v)
        return true;
    Graph* g = static_cast<CTPProblem*>(problem_)->roads();
    std::list<int> Q;
    Q.push_front(location_);
    std::unordered_set<int> visited;
    while (!Q.empty()) {
        int tmp = Q.front();
        Q.pop_front();
        visited.insert(tmp);
        std::unordered_map<int,double> neighbors = g->neighbors(tmp);
        for (std::pair<int, double> ne : neighbors) {
            int x = ne.first;
            if (status_[tmp][x] == ctp::BLOCKED)
                continue;
            if (visited.find(x) != visited.end())
                continue;
            if (x == v)
                return true;
            Q.push_front(x);
        }
    }
    return false;
}

bool CTPState::badWeather()
{
    if (badWeather_ != ctp::UNKNOWN)
        return (badWeather_ == ctp::TRUE) ? true : false;
    bool reachable =
        potentiallyReachable(
            static_cast<CTPProblem*>(problem_)->goalLocation());
    badWeather_ = reachable ? false : true;
    return !reachable;
}

double CTPState::distanceOpen(int v)
{
    Graph* g = static_cast<CTPProblem*>(problem_)->roads();
    std::vector<double> distances(g->numVertices(), gr_infinity);
    std::priority_queue<vertexCost,
                        std::vector<vertexCost>,
                        cmpVertexDijkstra> Q;
    Q.push(vertexCost(location_, 0.0));
    std::unordered_set<int> closed;
    while (!Q.empty()) {
        vertexCost uc = Q.top();
        Q.pop();
        distances[uc.vc_vertex] = std::min(uc.vc_cost, distances[uc.vc_vertex]);
        closed.insert(uc.vc_vertex);
        std::unordered_map<int,double> neighbors = g->neighbors(uc.vc_vertex);
        for (std::pair<int,double> vc : neighbors) {
            if (closed.find(vc.first) != closed.end()
                || status_[uc.vc_vertex][vc.first] != ctp::OPEN)
                continue;
            Q.push(vertexCost(vc.first, uc.vc_cost + vc.second));
        }
    }
    return distances[v];
}


double CTPState::distanceOptimistic(int v)
{
    Graph* g = static_cast<CTPProblem*>(problem_)->roads();
    std::vector<double> distances(g->numVertices(), gr_infinity);
    std::priority_queue<vertexCost,
                        std::vector<vertexCost>,
                        cmpVertexDijkstra> Q;
    Q.push(vertexCost(location_, 0.0));
    std::unordered_set<int> closed;
    while (!Q.empty()) {
        vertexCost uc = Q.top();
        Q.pop();
        distances[uc.vc_vertex] = std::min(uc.vc_cost, distances[uc.vc_vertex]);
        closed.insert(uc.vc_vertex);
        std::unordered_map<int,double> neighbors = g->neighbors(uc.vc_vertex);
        for (std::pair<int,double> vc : neighbors) {
            if (closed.find(vc.first) != closed.end()
                || status_[uc.vc_vertex][vc.first] == ctp::BLOCKED)
                continue;
            Q.push(vertexCost(vc.first, uc.vc_cost + vc.second));
        }
    }
    return distances[v];
}
