#ifndef MDPLIB_GWMANHATTANHEUR_H
#define MDPLIB_GWMANHATTANHEUR_H

#include "../../util/general.h"
#include "../../heuristic.h"
#include "../../problem.h"
#include "../../state.h"
#include "GridWorldProblem.h"
#include "GridWorldState.h"

class GWManhattanHeuristic : public Heuristic
{
public:
    GWManhattanHeuristic() {};

    virtual Rational cost(Problem* problem, State* s) const
    {
        GridWorldProblem* gwp = (GridWorldProblem*) problem;
        GridWorldState* gws = (GridWorldState*) s;

        Rational cost_(mdplib::dead_end_cost);
        for (PairRationalMap::iterator it = gwp->goals_->begin(); it != gwp->goals_->end(); it++) {
            std::pair<int,int> goal = it->first;
            Rational value = it->second;
            Rational md(abs(gws->x() - goal.first) + abs(gws->y() - goal.second));
            Rational goalCost = md * Rational(3, 100) + value;
            if (goalCost < cost_)
                cost_ = goalCost;
        }
        return cost_;
    }
};

#endif // MDPLIB_GWMANHATTANHEUR_H
