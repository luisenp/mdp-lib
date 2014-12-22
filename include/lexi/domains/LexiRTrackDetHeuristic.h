#ifndef MDPLIB_LEXIRTRACKDETHEURISTIC_H
#define MDPLIB_LEXIRTRACKDETHEURISTIC_H

#include "../../heuristic.h"

#include "RacetrackProblem.h"

namespace mllexi
{

/**
 * A deterministic heuristic for the Racetrack domain. The heuristic assumes that
 * for any action, the intended outcome will occur with probability 1.0; this
 * results in an admissible heuristic.
 */
class LexiRTrackDetHeuristic : public mlcore::Heuristic
{
private:
    LexiRacetrackProblem* detProblem_;
public:
    LexiRTrackDetHeuristic(char* filename);
    virtual ~LexiRTrackDetHeuristic()
    {
        delete detProblem_;
    }

    virtual double cost(const mlcore::State* s) const;
};

}

#endif // MDPLIB_LEXIRTRACKDETHEURISTIC_H
