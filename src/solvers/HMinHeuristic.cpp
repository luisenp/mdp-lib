#include "../include/State.h"

#include "../../include/solvers/HMinHeuristic.h"
#include "../../include/solvers/Solver.h"


using namespace mlcore;

namespace mlsolvers
{

HMinHeuristic::HMinHeuristic(Problem* problem, bool solveAll)
{
    solveAll_ = solveAll;
    if (solveAll_) {
        problem->generateAll();
        while (true) {
            double maxResidual = 0.0;
            for (State * s : problem->states()) {
                double prevCost = values_[s];
                double bestQ = mdplib::dead_end_cost;
                for (Action* a : problem->actions()) {
                    if (!problem->applicable(s, a))
                        continue;
                    double qAction = problem->cost(s, a);
                    double minCostSuccessor = mdplib::dead_end_cost;
                    for (auto const & successor : problem->transition(s, a)) {
                        minCostSuccessor =
                            std::min(minCostSuccessor,
                                     values_[successor.su_state]);
                    }
                    qAction += minCostSuccessor;
                    if (qAction <= bestQ)
                        bestQ = qAction;

                }
                maxResidual = std::max(maxResidual, fabs(bestQ - values_[s]));
                values_[s] = bestQ;
            }
            if (maxResidual < 1.0e-6)
                break;
        }
    }
}

double HMinHeuristic::cost(const State* s) const
{
    if (solveAll_) {
        auto it = values_.find(const_cast<State*> (s));
        assert(it != values_.end());
        return it->second;
    }
    return 0.0;
}

}
