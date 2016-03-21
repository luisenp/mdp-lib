#ifndef MDPLIB_HMINHEURISTIC_H
#define MDPLIB_HMINHEURISTIC_H


#include "../Heuristic.h"
#include "../Problem.h"


namespace mlsolvers
{

/**
 * Implements the hmin heuristic described in
 * http://www.aaai.org/Papers/ICAPS/2003/ICAPS03-002.pdf
 */
class HMinHeuristic : public mlcore::Heuristic
{
private:
    /*
     * If true, the heuristic value for all states will be found when the
     * the heuristic is constructed. Otherwise it will be computed on demand
     * TODO: Implement for solveAll_ = false;
     */
    bool solveAll_;

    /* Stores the computed values for the states. */
    mlcore::StateDoubleMap values_;

public:
    HMinHeuristic(mlcore::Problem* problem_, bool solveAll = true);

    virtual ~HMinHeuristic() { }

    virtual double cost(const mlcore::State* s) const;
};

}
#endif // MDPLIB_HMINHEURISTIC_H
