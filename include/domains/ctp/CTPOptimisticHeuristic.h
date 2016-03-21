#ifndef MDPLIB_CTPOPTIMHEUR_H
#define MDPLIB_CTPOPTIMHEUR_H

#include "CTPProblem.h"
#include "../../Heuristic.h"
#include "../../MDPLib.h"

#include "../../util/general.h"

class CTPOptimisticHeuristic : public mlcore::Heuristic
{
private:
    CTPProblem* problem_;
public:
    CTPOptimisticHeuristic();
    virtual ~CTPOptimisticHeuristic() {}

    CTPOptimisticHeuristic(CTPProblem* problem)
    {
        problem_ = problem;
    }
    virtual double cost(const mlcore::State* s)
    {
        CTPState* ctps = (CTPState* ) s;
        if (ctps->location() < 0)   // absorbing state
            return 0.0;
        double d = ctps->distanceOptimistic(problem_->goalLocation());
        if (d == gr_infinity)
            return 0.0;   // bad weather
        return d;
    }
};

#endif // MDPLIB_CTPOPTIMHEUR_H
