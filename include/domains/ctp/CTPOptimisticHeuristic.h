#ifndef MDPLIB_CTPOPTIMHEUR_H
#define MDPLIB_CTPOPTIMHEUR_H

#include "CTPProblem.h"
#include "../../heuristic.h"
#include "../../mdplib.h"

#include "../../util/general.h"

class CTPOptimisticHeuristic : public Heuristic
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
    virtual Rational cost(const State* s) const
    {
        CTPState* ctps = (CTPState* ) s;
        if (ctps->location() < 0)   // absorbing state
            return Rational(0.0);
        double d = ctps->distanceOptimistic(problem_->goalLocation());
        if (d == gr_infinity)
            return Rational(0.0);   // bad weather
        return Rational(d);
    }
};

#endif // MDPLIB_CTPOPTIMHEUR_H
