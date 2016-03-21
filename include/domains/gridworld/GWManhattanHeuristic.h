#ifndef MDPLIB_GWMANHATTANHEUR_H
#define MDPLIB_GWMANHATTANHEUR_H

#include "../../util/general.h"

#include "../../Heuristic.h"
#include "../../State.h"

#include "GridWorldProblem.h"
#include "GridWorldState.h"


class GWManhattanHeuristic : public mlcore::Heuristic
{
private:
    GridWorldProblem* problem_;

public:
    GWManhattanHeuristic(GridWorldProblem* problem)
    {
        problem_ = problem;
    }

    virtual double cost(const mlcore::State* s)
    {
        GridWorldState* gws = (GridWorldState*) s;
        double cost_ = mdplib::dead_end_cost;
        if (gws->x() == -1) // absorbing dummy state
            return 0;
        for (PairDoubleMap::iterator it = problem_->goals_->begin();
                it != problem_->goals_->end(); it++) {
            std::pair<int,int> goal = it->first;
            double value = it->second;
            if (gws->x() == goal.first && gws->y() == goal.second)
                return value;
            double md =
                abs(gws->x() - goal.first) + abs(gws->y() - goal.second);
            double goalCost = problem_->actionCost_ * md + value;
            if (goalCost < cost_)
                cost_ = goalCost;
        }
        return cost_;
    }
};

#endif // MDPLIB_GWMANHATTANHEUR_H
