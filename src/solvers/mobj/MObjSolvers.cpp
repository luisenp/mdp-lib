#include <cassert>
#include <cmath>

#include "../../include/solvers/Solver.h"
#include "../../include/solvers/mobj/MObjSolvers.h"

namespace mdplib_mobj_solvers
{

double qvalue(mlmobj::MOProblem* problem, mlmobj::MOState* s, mlcore::Action* a, int i)
{
    double qAction = 0.0;
    for (mlcore::Successor su : problem->transition(s, a, 0)) {
        qAction += su.su_prob * ((mlmobj::MOState *) su.su_state)->mobjCost()[i];
    }
    qAction = (qAction * problem->gamma()) + problem->cost(s, a, i);
    return qAction;
}


double lexiBellmanUpdate(mlmobj::MOProblem* problem, mlmobj::MOState* s, int level)
{
    bool hasAction = true;
    mlcore::Action* bestAction = nullptr;
    double residual = 0.0;
    if (problem->goal(s, 0)) {
        s->setBestAction(nullptr);
        for (int i = 0; i < problem->size(); i++)
            s->setCost(0.0, i);
        return 0.0;
    }

    std::list<mlcore::Action*> filteredActions = problem->actions();
    for (int i = 0; i <= level; i++) {
        std::vector<double> qActions(filteredActions.size());
        double bestQ = mdplib::dead_end_cost + 1;
        int actionIdx = 0;

        /* Computing Q-values for all actions w.r.t. the i-th cost function */
        for (mlcore::Action* a : filteredActions) {
            if (!problem->applicable(s, a))
                continue;
            qActions[actionIdx] = std::min(mdplib::dead_end_cost, qvalue(problem, s, a, i));
            if (qActions[actionIdx] < bestQ) {
                bestQ = qActions[actionIdx];
                bestAction = a;
            }
            actionIdx++;
        }

        /* Updating cost, best action and residual */
        double currentResidual = fabs(bestQ - s->mobjCost()[i]);
        if (currentResidual > residual)
            residual = currentResidual;
        s->setCost(bestQ, i);
        s->setBestAction(bestAction);
        if (bestQ > mdplib::dead_end_cost) {
            s->markDeadEnd();
            break;
        }

        /* Getting actions for the next lexicographic level */;
        if (i < level) {
            std::list<mlcore::Action*> prevActions = filteredActions;
            filteredActions.clear();
            actionIdx = 0;
            for (mlcore::Action* a : prevActions) {
                if (!problem->applicable(s, a))
                    continue;
                if (qActions[actionIdx] <= (bestQ + problem->slack() + 1.0e-8))
                    filteredActions.push_back(a);
                actionIdx++;
            }
        }
    }
    return residual;
}


double bellmanUpdate(mlmobj::MOProblem* problem, mlmobj::MOState* s)
{
    std::pair<double, mlcore::Action*> best = mlsolvers::bellmanBackup(problem, s);
    double residual = s->cost() - best.bb_cost;
    s->setCost(best.bb_cost);
    s->setBestAction(best.bb_action);
    for (int i = 0; i < problem->size(); i++)
        s->setCost(qvalue(problem, s, best.bb_action, i), i);
    return fabs(residual);
}


std::vector<double> sampleTrial(mlmobj::MOProblem* problem, mlcore::State* s)
{
    mlcore::State* tmp = s;
    double discount = 1.0;
    int k = problem->size();
    std::vector<double>mobjCost(k, 0.0);
    while (!problem->goal(tmp)) {
        mlcore::Action* a = tmp->bestAction();
        assert(a != nullptr);
        std::vector<double> discountedCost(k, 0.0);
        double totalDiscountedCostOfCurrentAction = 0.0;
        for (int i = 0; i < k; i++) {
            discountedCost[i] = discount * problem->cost(tmp, a, i);
            mobjCost[i] += discountedCost[i];
            totalDiscountedCostOfCurrentAction +=discountedCost[i];
        }
        if (totalDiscountedCostOfCurrentAction < 1.0-6)
            break;  // stop because no more cost can be accumulated (avoid infinite loop)
        tmp = mlsolvers::randomSuccessor(problem, tmp, a);
        discount *= problem->gamma();
    }
    return mobjCost;
}

} // mdplib_mobj_solvers
