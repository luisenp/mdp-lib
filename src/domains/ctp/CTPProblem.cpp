#include <cassert>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "../../../include/domains/ctp/CTPProblem.h"
#include "../../../include/domains/ctp/CTPAction.h"

#include "../../../include/util/general.h"

void CTPProblem::init()
{
    s0 = new CTPState(this, start_);
    absorbing_ = new CTPState(this, -1);
    this->addState(s0);
    this->addState(absorbing_);
    int n = roads_->numVertices();
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            actions_.push_back(new CTPAction(i, j));
        }
    }
    actions_.push_back(new CTPAction(-1,-1));
}

CTPProblem::CTPProblem(Graph* roads,
                       std::vector< std::vector <double> >& probs,
                       int start,
                       int goal)
   : roads_(roads), probs_(probs), start_(start), goal_(goal)
{
    init();
}

CTPProblem::CTPProblem(const char* filename)
{
    int nvertices, nedges;
    std::ifstream myfile (filename);
    if (myfile.is_open()) {
        char x;
        std::string line;
        std::getline(myfile, line);
        std::istringstream iss(line);
        iss >> x >> nvertices >> nedges;
        roads_ = new Graph(nvertices);
        for (int i = 0; i < nvertices; i++)
            probs_.push_back(std::vector<double> (nvertices));
        while ( std::getline (myfile, line) ) {
            std::istringstream iss(line);
            int u, v;
            double p, w;
            iss >> x >> u >> v >> p >> w;
            probs_[u - 1][v - 1] = probs_[v - 1][u - 1] = p;
            roads_->connect(u - 1, v - 1, w);
            roads_->connect(v - 1, u - 1, w);
        }
        myfile.close();
    } else {
        std::cerr << "Invalid file " << filename << std::endl;
        exit(-1);
    }
    start_ = 0;
    goal_ = nvertices - 1;

    init();
}

bool CTPProblem::goal(mlcore::State* s) const
{
    CTPState* ctps = static_cast<CTPState*>(s);
    if (ctps->badWeather())
        return true;
    return goal_ == ctps->location() ;
}

std::list<mlcore::Successor>
CTPProblem::transition(mlcore::State* s, mlcore::Action* a)
{
    assert(applicable(s, a));

    std::list<mlcore::Successor> successors;
    if (s == absorbing_) {
        successors.push_front(mlcore::Successor(s, 1.0));
        return successors;
    }

    if (goal(s)) {
        successors.push_front(mlcore::Successor(absorbing_, 1.0));
        return successors;
    }

    CTPState* ctps = static_cast<CTPState*>(s);
    CTPAction* ctpa = static_cast<CTPAction*>(a);
    int from = ctpa->from();
    int to = ctpa->to();

    std::vector<int> neighbors;
    for (std::pair<int, double> entry : roads_->neighbors(to)) {
        /* No need to go back to states that have already been explored */
        if (ctps->explored().find(entry.first) == ctps->explored().end())
            neighbors.push_back(entry.first);
    }
    int nadj = neighbors.size();
    for (int i = 0; i < (1 << nadj); i++) {
        CTPState* next = new CTPState(*ctps);
        next->setLocation(to);
        double p = 1.0;
        /* Updating adjacent roads */
        for (int j = 0; j < nadj; j++) {
            assert(ctps->status()[to][neighbors[j]] == ctp::UNKNOWN);
            unsigned char st = (i & (1<<j)) ? ctp::OPEN : ctp::BLOCKED;
            p *= (st == ctp::BLOCKED) ?
                    1.0 - probs_[to][neighbors[j]] :
                    probs_[to][neighbors[j]];
            next->setStatus(to, neighbors[j], st);
            next->setStatus(neighbors[j], to, st);
        }
        next->explored().insert(to);
        successors.push_back(mlcore::Successor(this->addState(next), p));
    }
    return successors;
}

double CTPProblem::cost(mlcore::State* s, mlcore::Action* a) const
{
    assert(applicable(s, a));
    if (s == absorbing_ || goal(s)) {
        return 0.0;
    }
    CTPState* ctps = static_cast<CTPState*>(s);
    CTPAction* ctpa = static_cast<CTPAction*>(a);
    return ctps->distanceOpen(ctpa->to());
}

bool CTPProblem::applicable(mlcore::State* s, mlcore::Action* a) const
{
    CTPState* ctps = static_cast<CTPState*>(s);
    CTPAction* ctpa = static_cast<CTPAction*>(a);
    if (s == absorbing_ || goal(s))
        return ctpa->from() == -1;
    if (ctps->location() != ctpa->from())
        return false;
    if (!ctps->reachable(ctpa->to()))
        return false;

    // Checking if the state was previously explored.
    // If so, no need to explore it again.
    return ( ctps->explored().find(ctpa->to())  == ctps->explored().end() );
}
