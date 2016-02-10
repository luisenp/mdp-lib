#ifndef MDPLIB_RTRACKLOWRESHEURISTIC_H
#define MDPLIB_RTRACKLOWRESHEURISTIC_H

#include "../../Heuristic.h"

#include "RacetrackProblem.h"



class RTrackLowResHeuristic : public mlcore::Heuristic
{
private:
    RacetrackProblem* lowResProblem_;

    int resolution_;
public:
    RTrackLowResHeuristic(char* filename,
                          int resolution,
                          double pSlip,
                          double pError);

    virtual ~RTrackLowResHeuristic();

    double cost(const mlcore::State* s) const;
};

#endif // MDPLIB_RTRACKLOWRESHEURISTIC_H
