#ifndef MDPLIB_AIRPLANEHEURISTIC_H
#define MDPLIB_AIRPLANEHEURISTIC_H

#include "../../../Heuristic.h"

#include "AirplaneProblem.h"

namespace mlmobj
{

class AirplaneHeuristic : public mlcore::Heuristic
{
private:
    AirplaneProblem* problem_;

    int level_;

public:

    AirplaneHeuristic(AirplaneProblem* problem, int level) :
        problem_(problem), level_(level) { }

    virtual ~AirplaneHeuristic() { }

    virtual double cost(const mlcore::State* s) const;
};

}

#endif // MDPLIB_AIRPLANEHEURISTIC_H
