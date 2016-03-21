#ifndef MDPLIB_MOGWSECONDHEUR_H
#define MDPLIB_MOGWSECONDHEUR_H

#include "../../util/general.h"
#include "../../heuristic.h"
#include "../../state.h"

#include "MOGridWorldProblem.h"
#include "MOGridWorldState.h"

namespace mlmobj
{

class MOGWManhattanHeuristic : public mlcore::Heuristic
{
private:
    MOGridWorldProblem* problem_;
    double costDown_;

public:
    MOGWManhattanHeuristic(MOGridWorldProblem* problem, double costDown)
    {
        problem_ = problem;
        costDown_ = costDown;
    }

    virtual double cost(const mlcore::State* s)
    {
        MOGridWorldState* gws = (MOGridWorldState*) s;
        double cost_ = mdplib::dead_end_cost;
        if (gws->x() == -1) // absorbing dummy state
            return 0;
        for (PairDoubleMap::iterator it = problem_->goals()[0].begin();
                                        it != problem_->goals()[0].end(); it++) {
            std::pair<int,int> goal = it->first;
            double value = it->second;
            if (gws->x() == goal.first && gws->y() == goal.second)
                return value;
            double distX = abs(gws->x() - goal.first);
            double distY = abs(gws->y() - goal.second);
            double mult = (gws->y() > goal.second) ? costDown_ : 1.0;
            double goalCost = problem_->actionCost() * (distX + mult * distY) + value;
            if (goalCost < cost_)
                cost_ = goalCost;
        }
        return cost_;
    }
};

}

#endif // MDPLIB_MOGWSECONDHEUR_H

