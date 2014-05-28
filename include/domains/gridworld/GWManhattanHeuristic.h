#ifndef MDPLIB_GWMANHATTANHEUR_H
#define MDPLIB_GWMANHATTANHEUR_H

#include "../../util/general.h"
#include "../../heuristic.h"
#include "../../state.h"
#include "GridWorldProblem.h"
#include "GridWorldState.h"

class GWManhattanHeuristic : public Heuristic
{
private:
    GridWorldProblem* problem_;

public:
    GWManhattanHeuristic(GridWorldProblem* problem)
    {
        problem_ = problem;
    }

    virtual Rational cost(const State* s) const
    {
        GridWorldState* gws = (GridWorldState*) s;
        Rational cost_(mdplib::dead_end_cost);
        if (gws->x() == -1) // absorbing dummy state
            return 0;
        for (PairRationalMap::iterator it = problem_->goals_->begin();
                                        it != problem_->goals_->end(); it++) {
            std::pair<int,int> goal = it->first;
            Rational value = it->second;
            if (gws->x() == goal.first && gws->y() == goal.second)
                return value;
            Rational md(abs(gws->x() - goal.first) + abs(gws->y() - goal.second));
            Rational goalCost = md * Rational(3, 100) + value;
            if (goalCost < cost_)
                cost_ = goalCost;
        }
        return cost_;
    }
};

#endif // MDPLIB_GWMANHATTANHEUR_H
