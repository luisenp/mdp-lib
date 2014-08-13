#include <vector>
#include <cassert>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>

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

        while ( std::getline (myfile, line) ) {
            grid_.push_back(std::vector<char> ());
            for (int i = 0; i < line.size(); i++) {
                grid_.back().push_back(line.at(i));
            }
        }
        myfile.close();
    }

    s0 = new RacetrackState(-1, -1, -1, -1, this);
    this->addState(s0);
    absorbing_ = new RacetrackState(-2, -2, -2, -2, this);
    this->addState(absorbing_);



                    for (int i = 0; i < grid_.size(); i++) {
                        for (int j = 0; j < grid_[i].size(); j++) {
                            std::cout << grid_[i][j];
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
    RacetrackState* rts = (RacetrackState*) s;
    RacetrackAction* rta = (RacetrackAction*) a;

    int idAction = rta->hashValue();
    all_successor_t* allSuccessors = rts->allSuccessors();

    if (!allSuccessors->at(idAction).empty()) {
        return allSuccessors->at(idAction);
    }



    return allSuccessors->at(idAction);
}

double RacetrackProblem::cost(mlcore::State* s, mlcore::Action* a) const
{
    if (s == absorbing_ || goal(s))
        return 0.0;

    return 1.0;
}

bool RacetrackProblem::applicable(mlcore::State* s, mlcore::Action* a) const
{
    return true;
}
