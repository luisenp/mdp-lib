#ifndef MDPLIB_AODETHEURISTIC_H
#define MDPLIB_AODETHEURISTIC_H


#include "../Heuristic.h"
#include "../Problem.h"


namespace mlsolvers
{

/**
 * Implements the optimal solution to the all-outcomes determinization.
 */
class AODetHeuristic : public mlcore::Heuristic {
private:
    /* The problem for which this heuristic is computed. */
    mlcore::Problem* problem_;

    /* Stores the computed costs. */
    mlcore::StateDoubleMap costs_;

public:
    AODetHeuristic(mlcore::Problem* problem);

    virtual ~AODetHeuristic() { }

    virtual double cost(const mlcore::State* s);
};

}
#endif // MDPLIB_AODETHEURISTIC_H
