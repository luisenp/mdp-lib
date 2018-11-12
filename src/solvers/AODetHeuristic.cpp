#include <cassert>

#include "../../include/domains/AODeterminization.h"
#include "../../include/solvers/AODetHeuristic.h"
#include "../../include/solvers/VISolver.h"


using namespace mlcore;
using namespace std;

namespace mlsolvers
{

AODetHeuristic::AODetHeuristic(mlcore::Problem* problem, bool precompute) {
    if (precompute) {
        aodet_ = new AllOutcomesDeterminization(problem);
        solveVI();
        for (State* s : problem->states()) {
            costs_[s] = s->cost();
        }
    } else {
        solver_ = new DeterministicSolver(problem, det_all_outcomes);
    }
}

void AODetHeuristic::solveVI(int maxIter, double tol) {
    auto beginTime = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < maxIter; i++) {
        double maxResidual = 0.0;
        for (mlcore::State* s : aodet_->states()) {
            if (aodet_->goal(s)) {
                continue;
            }
            double residual = AODetBellmanUpdate(s);
            maxResidual = std::max(residual, maxResidual);
        }
        if (maxResidual < tol)
            return;
    }
    return;
}

double AODetHeuristic::AODetBellmanUpdate(mlcore::State* s) {

    double bestQ = aodet_->goal(s) ? 0.0 : mdplib::dead_end_cost;
    bool hasAction = false;
    mlcore::Action* bestAction = nullptr;
    for (mlcore::Action* a : aodet_->actions(s)) {
        if (!aodet_->applicable(s, a))
            continue;
        hasAction = true;
        double qAction = std::min(mdplib::dead_end_cost, qvalue(aodet_, s, a));
        if (qAction <= bestQ) {
            bestQ = qAction;
            bestAction = a;
        }
    }
    if (!hasAction && bestQ >= mdplib::dead_end_cost)
        s->markDeadEnd();
    double residual = s->cost() - bestQ;
    s->setCost(bestQ);
    s->setBestAction(bestAction);
    return fabs(residual);
}


double AODetHeuristic::cost(const State* s) {
    State* ss = const_cast<State*> (s);
    if (costs_.count(ss) == 0) {
//                                                                                dprint("here-1", ss);
        solver_->solve(ss);
        mlcore::StateDoubleMap& pathCosts = solver_->costLastPathFound();
        for (auto& stateCost : pathCosts) {
            costs_[stateCost.first] = stateCost.second;
//                                                                                dprint("here-2", stateCost.first, costs_.at(stateCost.first));
        }
    }
//                                                                                dprint(costs_.at(ss));
    return costs_.at(ss);
}

} // namespace mlsovers
