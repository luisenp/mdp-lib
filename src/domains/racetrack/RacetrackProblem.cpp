#include <vector>
#include <cassert>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>

#include "../../../include/domains/racetrack/RacetrackProblem.h"
#include "../../../include/domains/racetrack/RacetrackState.h"
#include "../../../include/domains/racetrack/RacetrackAction.h"


RacetrackProblem::RacetrackProblem(const char* filename)
{
    std::ifstream myfile (filename);
    int width, height;
    if (myfile.is_open()) {
        std::string line;
        int x = 0, y;

        std::getline(myfile, line);
        std::istringstream iss(line);
        iss >> x;
        std::getline(myfile, line);
        iss.str(line); iss.clear();
        iss >> y;

        track_ =
            std::vector <std::vector<char> > (x + 2,
                                              std::vector<char> (y + 2, 'X'));

        while ( std::getline (myfile, line) ) {
            for (int i = 0; i < line.size(); i++) {
                if (!rtrack::checkValid(line.at(i)))
                    continue;
                track_[i + 1][y] = line.at(i);
                if (line.at(i) == rtrack::start) {
                    starts_.insert(std::pair<int,int> (i + 1, y));
                }
                if (line.at(i) == rtrack::goal) {
                    goals_.insert(std::pair<int,int> (i + 1, y));
                }
            }
            y--;
        }
        myfile.close();
    } else {
        std::cerr << "Invalid file " << filename << std::endl;
        exit(-1);
    }

    s0 = new RacetrackState(-1, -1, -1, -1, this);
    this->addState(s0);
    absorbing_ = new RacetrackState(-2, -2, -2, -2, this);
    this->addState(absorbing_);

    for (int ax = -1; ax <= 1; ax++)
        for (int ay = -1; ay <= 1; ay++)
        actions_.push_back(new RacetrackAction(ax, ay));
}


void RacetrackProblem::printTrack(std::ostream& os)
{
    for (int i = 0; i < track_.size(); i++) {
        for (int j = 0; j < track_[i].size(); j++) {
            os << track_[i][j];
        }
        os << std::endl;
    }
}


bool RacetrackProblem::goal(mlcore::State* s) const
{
    if (s == absorbing_)
        return true;
    RacetrackState* rts = static_cast<RacetrackState*>(s);
    std::pair<int, int> pos(rts->x(), rts->y());
    return goals_.find(pos) != goals_.end();
}


std::list<mlcore::Successor>
RacetrackProblem::transition(mlcore::State* s, mlcore::Action* a)
{
    if (useFlatTransition_)
        return flatTransition(s, a);

    assert(applicable(s, a));

    if (s == s0) {
        std::list<mlcore::Successor> successors;
        for (std::pair<int,int> start : starts_) {
            mlcore::State* next =
                new RacetrackState(start.first, start.second, 0, 0, this);
            successors.push_back(mlcore::Successor(this->addState(next),
                                 1.0 / starts_.size()));
        }
        return successors;
    }

    if (goal(s) || s == absorbing_) {
        std::list<mlcore::Successor> successors;
        successors.push_back(
            mlcore::Successor(this->addState(absorbing_), 1.0));
        return successors;
    }

    RacetrackState* rts = static_cast<RacetrackState*>(s);
    RacetrackAction* rta = static_cast<RacetrackAction*>(a);

    int idAction = rta->hashValue();
    std::vector<mlcore::SuccessorsList>* allSuccessors = rts->allSuccessors();

    if (!allSuccessors->at(idAction).empty()) {
        return allSuccessors->at(idAction);
    }

    /* At walls the car can deterministically move to the track again */
    if (track_[rts->x()][rts->y()] == rtrack::wall ||
            track_[rts->x()][rts->y()] == rtrack::pothole) {
        int x = rts->x(), y = rts->y();
        int ax = rta->ax(), ay = rta->ay();
        mlcore::State* next =
          this->addState(new RacetrackState(x + ax, y + ay, ax, ay, this));
        allSuccessors->at(idAction).push_back(mlcore::Successor(next, 1.0));
        return allSuccessors->at(idAction);
    }

    bool isDet = (abs(rts->vx()) + abs(rts->vy())) < mds_;
    bool isErr = track_[rts->x()][rts->y()] == rtrack::error;
    double p_err = isDet ? 0.0 : pError_ * (1 - pSlip_);
    double p_slip = isDet ? 0.0 : pSlip_;
    double p_int = isDet ? 1.0 : (1.0 - pSlip_) * (1.0 - pError_);

    if (!isErr) {
        p_int = 1.0 - p_slip;
        p_err = 0.0;
    }

    double acc = 0.0;
    if (p_slip != 0.0) {
        mlcore::State* next = this->addState(resultingState(rts, 0, 0));
        allSuccessors->at(idAction).push_back(mlcore::Successor(next, pSlip_));
        acc += pSlip_;
    }
    if (p_int != 0.0) {
        mlcore::State* next =
            this->addState(resultingState(rts, rta->ax(), rta->ay()));
        allSuccessors->at(idAction).push_back(mlcore::Successor(next, p_int));
        acc += p_int;
    }
    if (p_err != 0.0) {
        /*
         * "ta" stores how many other actions are within distance 1 of
         * the current action.
         */
        int ta = abs(rta->ax()) + abs(rta->ay());
        int cnt = 4;
        if (ta == 1) cnt = 3;
        if (ta == 2) cnt = 2;
        for (mlcore::Action* a2 : actions_) {
            RacetrackAction* rtaE = static_cast<RacetrackAction*>(a2);
            int dist =
                abs(rtaE->ax() - rta->ax()) + abs(rtaE->ay() - rta->ay());
            if (dist == 0 || dist > 1)
                continue;
            mlcore::State* next =
                this->addState(resultingState(rts, rtaE->ax(), rtaE->ay()));
            allSuccessors->at(idAction).
                push_back(mlcore::Successor(next, p_err / cnt));
            acc += p_err / cnt;
        }
    }

    assert(fabs(acc - 1.0) < 1.0e-6);

    return allSuccessors->at(idAction);
}


double RacetrackProblem::cost(mlcore::State* s, mlcore::Action* a) const
{
    if (s == s0 || s == absorbing_ || goal(s))
        return 0.0;

    RacetrackState* rts = static_cast<RacetrackState*>(s);
    if (track_[rts->x()][rts->y()] == rtrack::wall)
        return 10.0;

    if (track_[rts->x()][rts->y()] == rtrack::pothole)
        return 100.0;

    return 1.0;
}


bool RacetrackProblem::applicable(mlcore::State* s, mlcore::Action* a) const
{
    RacetrackState* rts = static_cast<RacetrackState*>(s);
    RacetrackAction* rta = static_cast<RacetrackAction*>(a);

    if (s == s0 || s == absorbing_) {
        // All actions do the same, so just allow one of them
        return a == actions_.front();
    }

    int x = rts->x() + rta->ax(), y = rts->y() + rta->ay();

    if (x < 0 || x >= track_.size() || y < 0 || y >= track_[x].size())
        return false;

    if (track_[rts->x()][rts->y()] == rtrack::wall &&
            track_[x][y] == rtrack::wall)
        return false;
    if (track_[rts->x()][rts->y()] == rtrack::pothole &&
            track_[x][y] == rtrack::pothole)
    return false;

    return true;
}


RacetrackState*
RacetrackProblem::resultingState(RacetrackState* rts, int ax, int ay)
{
    int x1 = rts->x(), y1 = rts->y();
    int vx = rts->vx() + ax, vy = rts->vy() + ay;

    int m = 2 * (abs(vx) + abs(vy));

    if (m == 0)
        return new RacetrackState(x1, y1, 0, 0, this);

    for (int d = 0; d <= m; d++) {
        int x2 = round(x1 + (double) (d * vx) / m);
        int y2 = round(y1 + (double) (d * vy) / m);
        if (track_[x2][y2] == rtrack::wall ||
                track_[x2][y2] == rtrack::pothole) {
            return new RacetrackState(x2, y2, 0, 0, this);
        }
        if (track_[x2][y2] == rtrack::goal) {
            return new RacetrackState(x2, y2, vx, vy, this);
        }
    }
    return new RacetrackState(x1 + vx, y1 + vy, vx, vy, this);
}


std::list<mlcore::Successor>
RacetrackProblem::flatTransition(mlcore::State* s, mlcore::Action* a)
{
    assert(applicable(s, a));

    RacetrackState* rts = static_cast<RacetrackState*>(s);
    RacetrackAction* rta = static_cast<RacetrackAction*>(a);

    if (s == s0) {
        std::list<mlcore::Successor> successors;
        for (std::pair<int,int> start : starts_) {
            mlcore::State* next =
                new RacetrackState(start.first, start.second, 0, 0, this);
            successors.push_back(mlcore::Successor(this->addState(next),
                                 1.0 / starts_.size()));
        }
        return successors;
    }

    int numSuccessors = numSuccessorsAction(rta);

    if (goal(s) || s == absorbing_) {
        std::list<mlcore::Successor> successors;
        for (int i = 0; i < numSuccessors; i++)
            successors.push_back(
                mlcore::Successor(this->addState(absorbing_),
                                  1.0 / numSuccessors));
        return successors;
    }

    int idAction = rta->hashValue();
    std::vector<mlcore::SuccessorsList>* allSuccessors = rts->allSuccessors();
    if (!allSuccessors->at(idAction).empty()) {
        return allSuccessors->at(idAction);
    }

    /* At walls the car can deterministically move to the track again */
    if (track_[rts->x()][rts->y()] == rtrack::wall ||
            track_[rts->x()][rts->y()] == rtrack::pothole) {
        int x = rts->x(), y = rts->y();
        int ax = rta->ax(), ay = rta->ay();
        mlcore::State* next =
          this->addState(new RacetrackState(x + ax, y + ay, ax, ay, this));
        for (int i = 0; i < numSuccessors; i++)
            allSuccessors->at(idAction).
                push_back(mlcore::Successor(next, 1.0 / numSuccessors));
        return allSuccessors->at(idAction);
    }

    bool isDet = (abs(rts->vx()) + abs(rts->vy())) < mds_;
    bool isErr = track_[rts->x()][rts->y()] == rtrack::error;
    double p_err = isDet ? 0.0 : pError_ * (1 - pSlip_);
    double p_slip = isDet ? 0.0 : pSlip_;
    double p_int = isDet ? 1.0 : (1.0 - pSlip_) * (1.0 - pError_);

    if (!isErr) {
        p_int = 1.0 - p_slip;
        p_err = 0.0;
    }

    double acc = 0.0;
    mlcore::State* next = this->addState(resultingState(rts, 0, 0));
    allSuccessors->at(idAction).push_back(mlcore::Successor(next, p_slip));
    acc += p_slip;

    next = this->addState(resultingState(rts, rta->ax(), rta->ay()));
    allSuccessors->at(idAction).push_back(mlcore::Successor(next, p_int));
    acc += p_int;

    for (mlcore::Action* a2 : actions_) {
        RacetrackAction* rtaE = static_cast<RacetrackAction*>(a2);
        int dist =
            abs(rtaE->ax() - rta->ax()) + abs(rtaE->ay() - rta->ay());
        if (dist == 0 || dist > 1)
            continue;
        next = this->addState(resultingState(rts, rtaE->ax(), rtaE->ay()));
        allSuccessors->at(idAction).
            push_back(mlcore::Successor(next, p_err / (numSuccessors - 2)));
        acc += p_err / (numSuccessors - 2);
    }
    assert(fabs(acc - 1.0) < 1.0e-6);

    return allSuccessors->at(idAction);
}


int RacetrackProblem::numSuccessorsAction(RacetrackAction* rta)
{
    int actionMagnitude = abs(rta->ax()) + abs(rta->ay());
    if (actionMagnitude == 1)
        return 5;
    else if (actionMagnitude == 2)
        return 4;
    return 6;

}

