#ifndef MDPLIB_RTRACKDETHEURISTIC_H
#define MDPLIB_RTRACKDETHEURISTIC_H

#include "../../Heuristic.h"

#include "RacetrackProblem.h"

/**
 * A deterministic heuristic for the Racetrack domain.
 * The heuristic assumes that for any action, the intended outcome
 * will occur with probability 1.0; this results in an admissible heuristic.
 */
class RTrackDetHeuristic : public mlcore::Heuristic
{
private:
    RacetrackProblem* detProblem_;
public:
    RTrackDetHeuristic(const char* filename);
    virtual ~RTrackDetHeuristic()
    {
        delete detProblem_;
    }

    virtual double cost(const mlcore::State* s);
};

#endif // MDPLIB_RTRACKDETHEURISTIC_H
