#ifndef MDPLIB_MORTRACKDETHEURISTIC_H
#define MDPLIB_MORTRACKDETHEURISTIC_H

#include "../../Heuristic.h"

#include "MORacetrackProblem.h"

namespace mlmobj
{

/**
 * A deterministic heuristic for the Racetrack domain. The heuristic assumes that
 * for any action, the intended outcome will occur with probability 1.0; this
 * results in an admissible heuristic.
 */
class MORTrackDetHeuristic : public mlcore::Heuristic
{
private:
    MORacetrackProblem* detProblem_;
public:
    MORTrackDetHeuristic(char* filename, bool useSafety);
    virtual ~MORTrackDetHeuristic()
    {
        delete detProblem_;
    }

    virtual double cost(const mlcore::State* s);
};

}

#endif // MDPLIB_MORTRACKDETHEURISTIC_H
