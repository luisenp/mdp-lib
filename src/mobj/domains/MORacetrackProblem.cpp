#include <vector>
#include <cassert>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>

#include "../../../include/mobj/domains/MORacetrackProblem.h"
#include "../../../include/mobj/domains/MORacetrackState.h"

#include "../../../include/domains/racetrack/RacetrackProblem.h"
#include "../../../include/domains/racetrack/RacetrackAction.h"

namespace mlmobj
{

MORacetrackProblem::MORacetrackProblem(char* filename, int size)
{
    size_ = size;
    weights_ = std::vector<double> (size_, 0.0);
    weights_[0] = 1.0;

    std::ifstream myfile (filename);
    int width, height;
    if (myfile.is_open()) {
        std::string line;
        int x = -1, y = -1;

        std::getline(myfile, line);
        std::istringstream iss(line);
        iss >> x;
        std::getline(myfile, line);
        iss.clear(); iss.str(line);
        iss >> y;

        track_ = std::vector <std::vector<char> > (x, std::vector<char> (y));
        while ( std::getline (myfile, line) ) {
            y--;
            for (int i = 0; i < line.size(); i++) {
                track_[i][y] = line.at(i);
                if (line.at(i) == rtrack::start) {
                    starts_.insert(std::pair<int,int> (i, y));
                }
                if (line.at(i) == rtrack::goal) {
                    goals_.insert(std::pair<int,int> (i, y));
                }
            }
        }
        myfile.close();
    }

    s0 = new MORacetrackState(-1, -1, -1, -1, true, this);
    this->addState(s0);
    absorbing_ = new MORacetrackState(-2, -2, -2, -2, true, this);
    this->addState(absorbing_);

    for (int ax = -1; ax <= 1; ax++)
        for (int ay = -1; ay <= 1; ay++)
        actions_.push_back(new RacetrackAction(ax, ay));
}


void MORacetrackProblem::printTrack(std::ostream& os)
{
    for (int i = 0; i < track_.size(); i++) {
        for (int j = 0; j < track_[i].size(); j++) {
            os << track_[i][j];
        }
        os << std::endl;
    }
}


bool MORacetrackProblem::goal(mlcore::State* s, int index) const
{
    MORacetrackState* rts = (MORacetrackState*) s;
    std::pair<int, int> pos(rts->x(), rts->y());
    return goals_.find(pos) != goals_.end();
}


std::list<mlcore::Successor>
MORacetrackProblem::transition(mlcore::State* s, mlcore::Action* a, int index)
{
    assert(applicable(s, a));

    if (s == s0) {
        std::list<mlcore::Successor> successors;
        for (std::pair<int,int> start : starts_) {
            mlcore::State* next =
                new MORacetrackState(start.first, start.second, 0, 0, true, this);
            successors.push_back(mlcore::Successor(this->addState(next), 1.0 / starts_.size()));
        }
        return successors;
    }

    if (goal(s) || s == absorbing_) {
        std::list<mlcore::Successor> successors;
        successors.push_back(mlcore::Successor(this->addState(absorbing_), 1.0));
        return successors;
    }

    MORacetrackState* rts = (MORacetrackState*) s;
    RacetrackAction* rta = (RacetrackAction*) a;

    int idAction = rta->hashValue();
    std::vector<mlcore::SuccessorsList>* allSuccessors = rts->allSuccessors();

    if (!allSuccessors->at(idAction).empty()) {
        return allSuccessors->at(idAction);
    }

    /* At walls the car can deterministically move to the track again */
    if (track_[rts->x()][rts->y()] == rtrack::wall) {
        int x = rts->x(), y = rts->y();
        int ax = rta->ax(), ay = rta->ay();
        mlcore::State* next =
            this->addState(new MORacetrackState(x + ax, y + ay, ax, ay, true, this));
        allSuccessors->at(idAction).push_back(mlcore::Successor(next, 1.0));
        return allSuccessors->at(idAction);
    }

    /* Computing transition probabilities */
    bool isDet = (abs(rts->vx()) + abs(rts->vy())) < mds_;
    bool isErr = track_[rts->x()][rts->y()] == rtrack::error;
    double p_err = isDet ? 0.0 : pError_*(1 - pSlip_);
    double p_slip = isDet ? 0.0 : pSlip_;
    double p_int = isDet ? 1.0 : (1.0 - pSlip_)*(1.0 - pError_);
    if (!isErr) {
        p_int = 1.0 - p_slip;
        p_err = 0.0;
    }

    if (p_slip != 0.0) {
        addSuccessors(idAction, rts, resultingState(rts, 0, 0), pSlip_);
    }
    if (p_int != 0.0) {
        addSuccessors(idAction, rts, resultingState(rts, rta->ax(), rta->ay()), p_int);
    }
    if (p_err != 0.0) {
        /* "ta" stores how many other actions are within distance 1 of the current action */
        int ta = abs(rta->ax()) + abs(rta->ay());
        int cnt = 4;
        if (ta == 1) cnt = 3;
        if (ta == 2) cnt = 2;
        for (mlcore::Action* a2 : actions_) {
            RacetrackAction* rtaE = (RacetrackAction*) a2;
            int dist = abs(rtaE->ax() - rta->ax()) + abs(rtaE->ay() - rta->ay());
            if (dist == 0 || dist > 1)
                continue;
            addSuccessors(idAction, rts, resultingState(rts, rtaE->ax(), rtaE->ay()), p_err / cnt);
        }
    }

    return allSuccessors->at(idAction);
}


double MORacetrackProblem::cost(mlcore::State* s, mlcore::Action* a, int index) const
{
    if (s == s0 || s == absorbing_ || goal(s))
        return 0.0;
    MORacetrackState* rts = (MORacetrackState*) s;
    if (track_[rts->x()][rts->y()] == rtrack::wall)
        return 10.0;

    if (index == 0) {
        return 1.0;
    } else {
        RacetrackAction* rta = (RacetrackAction*) a;

        if ( (useSafety_ && index == 1) || (!useSafety_ && index == 2) )
            return (rts->safe()) ? 1.0 : 10.0;

        /* Keeping current speed or starting up the car has normal cost */
        if ( ( ( abs(rta->ax()) + abs(rta->ay()) ) ==  0) ||
             ( ( abs(rts->vx()) + abs(rts->vy()) ) ==  0) )
            return 1.0;

        /* Otherwise, the cost is related how sharp is the attempted turn */
        double m1 = sqrt ( rts->vx() * rts->vx() + rts->vy() * rts->vy() );
        double m2 = sqrt ( rta->ax() * rta->ax() + rta->ay() * rta->ay() );
        double dot = (rts->vx() * rta->ax() + rts->vy() * rta->ay()) / (m1 * m2);
        double angle = acos( dot );

        return 2 * fabs(angle) + 1.0;
    }
}


bool MORacetrackProblem::applicable(mlcore::State* s, mlcore::Action* a) const
{
    MORacetrackState* rts = (MORacetrackState*) s;
    RacetrackAction* rta = (RacetrackAction*) a;

    if (s == s0 || s == absorbing_)
        return true;

    int x = rts->x() + rta->ax(), y = rts->y() + rta->ay();

    if (x < 0 || x >= track_.size() || y < 0 || y >= track_[x].size())
        return false;

    if (track_[rts->x()][rts->y()] == rtrack::wall && track_[x][y] == rtrack::wall)
        return false;

    return true;
}


MORacetrackState* MORacetrackProblem::resultingState(MORacetrackState* rts, int ax, int ay)
{
    int x1 = rts->x(), y1 = rts->y();
    int vx = rts->vx() + ax, vy = rts->vy() + ay;

    int m = 2 * (abs(vx) + abs(vy));

    if (m == 0)
        return new MORacetrackState(x1, y1, 0, 0, isSafeLocation(x1, y1), this);

    for (int d = 0; d <= m; d++) {
        int x2 = round(x1 + (double) (d * vx) / m);
        int y2 = round(y1 + (double) (d * vy) / m);
        if (track_[x2][y2] == rtrack::wall) {
            return new MORacetrackState(x2, y2, 0, 0, true, this);
        }
        if (track_[x2][y2] == rtrack::goal) {
            return new MORacetrackState(x2, y2, vx, vy, true, this);
        }
    }
    return new MORacetrackState(x1 + vx, y1 + vy, vx, vy,
                                   isSafeLocation(x1 + vx, y1 + vy), this);
}

bool MORacetrackProblem::isSafeLocation(int x, int y)
{
    return (track_[x][y] != unsafeChar_);
}


void MORacetrackProblem::addSuccessors(int idAction,
                                          MORacetrackState* source,
                                          MORacetrackState* dest,
                                          double prob)
{
    std::vector<mlcore::SuccessorsList>* allSuccessors = source->allSuccessors();
    if (useSafety_ && !source->safe() && dest->safe()) {
        MORacetrackState* dest2 =
            new MORacetrackState(dest->x(), dest->y(), dest->vx(), dest->vy(), false, this);
        mlcore::State* next1 = this->addState(dest);
        mlcore::State* next2 = this->addState(dest2);
        allSuccessors->at(idAction).push_back(mlcore::Successor(next1, prob * 0.9));
        allSuccessors->at(idAction).push_back(mlcore::Successor(next2, prob * 0.1));
    } else {
        mlcore::State* next = this->addState(dest);
        allSuccessors->at(idAction).push_back(mlcore::Successor(next, prob));
    }
}

}
