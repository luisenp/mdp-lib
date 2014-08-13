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

RacetrackProblem::RacetrackProblem(char* filename)
{
    std::ifstream myfile (filename);
    std::istringstream iss;
    int width, height;
    if (myfile.is_open()) {
        std::string line;

        std::getline (myfile, line);
        std::getline (myfile, line);

        int y = 0;
        while ( std::getline (myfile, line) ) {
            track_.push_back(std::vector<char> ());
            for (int x = 0; x < line.size(); x++) {
                if (!rtrack::checkValid(line.at(x)))
                    continue;
                track_.back().push_back(line.at(x));
                if (line.at(x) == rtrack::start) {
                    starts_.insert(std::pair<int,int> (y, x));
                }
                if (line.at(x) == rtrack::goal) {
                    goals_.insert(std::pair<int,int> (y, x));
                    dprint2(y,x);
                }
            }
            y++;
        }
        myfile.close();
    }

    s0 = new RacetrackState(-1, -1, -1, -1, this);
    this->addState(s0);
    absorbing_ = new RacetrackState(-2, -2, -2, -2, this);
    this->addState(absorbing_);

    for (int ax = -1; ax <= 1; ax++)
        for (int ay = -1; ay <= 1; ay++)
        actions_.push_back(new RacetrackAction(ax, ay));

                    for (int i = 0; i < track_.size(); i++) {
                        for (int j = 0; j < track_[i].size(); j++) {
                            std::cout << track_[i][j];
                        }
                        std::cout << std::endl;
                    }
}

bool RacetrackProblem::goal(mlcore::State* s) const
{
    RacetrackState* rts = (RacetrackState*) s;
    std::pair<int, int> pos(rts->x(), rts->y());
    return goals_.find(pos) != goals_.end();
}

std::list<mlcore::Successor> RacetrackProblem::transition(mlcore::State* s, mlcore::Action* a)
{
    assert(applicable(s, a));

    if (s == s0) {
        std::list<mlcore::Successor> successors;
        for (std::pair<int,int> start : starts_) {
            mlcore::State* next = new RacetrackState(start.first, start.second, 0, 0, this);
            successors.push_back(mlcore::Successor(this->addState(next), 1.0 / starts_.size()));
        }
        return successors;
    }

    if (goal(s) || s == absorbing_) {
        std::list<mlcore::Successor> successors;
        successors.push_back(mlcore::Successor(this->addState(absorbing_), 1.0));
        return successors;
    }

    RacetrackState* rts = (RacetrackState*) s;
    RacetrackAction* rta = (RacetrackAction*) a;

    int idAction = rta->hashValue();
    all_successor_t* allSuccessors = rts->allSuccessors();

    if (!allSuccessors->at(idAction).empty()) {
        return allSuccessors->at(idAction);
    }

    /* At walls the car can deterministically move to the track again */
    if (track_[rts->x()][rts->y()] == rtrack::wall) {
        int x = rts->x(), y = rts->y();
        int ax = rta->ax(), ay = rta->ay();
        mlcore::State* next = this->addState(new RacetrackState(x + ax, y + ay, ax, ay, this));
        allSuccessors->at(idAction).push_back(mlcore::Successor(next, 1.0));
        return allSuccessors->at(idAction);
    }

    if (pSlip_ != 0.0) {
        mlcore::State* next = this->addState(resultingState(rts, 0, 0));
        allSuccessors->at(idAction).push_back(mlcore::Successor(next, pSlip_));
    }
    if (pSlip_ != 1.0) {
        mlcore::State* next = this->addState(resultingState(rts, rta->ax(), rta->ay()));
        allSuccessors->at(idAction).push_back(mlcore::Successor(next, 1.0 - pSlip_));
    }

    return allSuccessors->at(idAction);
}

double RacetrackProblem::cost(mlcore::State* s, mlcore::Action* a) const
{
    if (s == s0 || s == absorbing_ || goal(s))
        return 0.0;

    RacetrackState* rts = (RacetrackState*) s;
    if (track_[rts->x()][rts->y()] == rtrack::wall)
        return 10.0;

    return 1.0;
}

bool RacetrackProblem::applicable(mlcore::State* s, mlcore::Action* a) const
{
    RacetrackState* rts = (RacetrackState*) s;
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

RacetrackState* RacetrackProblem::resultingState(RacetrackState* rts, int ax, int ay)
{
    int x1 = rts->x(), y1 = rts->y();
    int vx = rts->vx() + ax, vy = rts->vy() + ay;

    int m = 2 * (abs(vx) + abs(vy));

    if (m == 0)
        return new RacetrackState(x1, y1, 0, 0, this);

    for (int d = 0; d <= m; d++) {
        int x2 = round(x1 + (double) (d * vx) / m);
        int y2 = round(y1 + (double) (d * vy) / m);
        if (track_[x2][y2] == rtrack::wall || track_[x2][y2] == rtrack::goal) {
            return new RacetrackState(x2, y2, 0, 0, this);
        }
    }
    return new RacetrackState(x1 + vx, y1 + vy, vx, vy, this);
}
