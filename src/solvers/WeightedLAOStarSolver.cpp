#include "../../include/problem.h"
#include "../../include/state.h"
#include "../../include/solvers/solver.h"
#include "../../include/solvers/WeightedLAOStarSolver.h"

#include "../../include/util/general.h"

#include <ctime>

namespace mlsolvers
{

double WeightedLAOStarSolver::bellmanUpdate(mlcore::State* s)
{
    double bestQ = problem_->goal(s) ? 0.0 : mdplib::dead_end_cost;
    double bestG = bestQ;
    double bestH = bestQ;
    bool hasAction = false;
    mlcore::Action* bestAction = nullptr;
    for (mlcore::Action* a : problem_->actions()) {
        if (!problem_->applicable(s, a))
            continue;
        hasAction = true;

        double actionG = problem_->cost(s, a);
        double actionH = 0.0;
        for (mlcore::Successor su : problem_->transition(s, a)) {
            mlcore::State* s = su.su_state;
            actionG += problem_->gamma() * su.su_prob * gValues_[s];
            actionH += problem_->gamma() * su.su_prob * hValues_[s];
        }
        double qAction = actionG + weight_ * actionH;
        if (qAction < bestQ) {
            bestQ = qAction;
            bestG = actionG;
            bestH = actionH;
            bestAction = a;
        }
    }

    if (!hasAction && bestQ == mdplib::dead_end_cost)
        s->markDeadEnd();

    double residual = bestQ - (gValues_[s] + weight_ * hValues_[s]);
    hValues_[s] = bestH;
    gValues_[s] = bestG;
    bestActions_[s] = bestAction;
    s->setCost(bestQ);
    s->setBestAction(bestAction);
    return fabs(residual);
}

mlcore::Action* WeightedLAOStarSolver::solve(mlcore::State* s0)
{
    clock_t startTime = clock();
    int totalExpanded = 0;
    int countExpanded = 0;
    double error = mdplib::dead_end_cost;
    while (true) {
        do {
            visited.clear();
            countExpanded = expand(s0);
            totalExpanded += countExpanded;

            clock_t currentTime = clock();
            if ((0.001 * (currentTime - startTime)) / CLOCKS_PER_SEC > timeLimit_)
                return bestActions_[s0];

        } while (countExpanded != 0);

        while (true) {

            clock_t currentTime = clock();
            if ((0.001 * (currentTime - startTime)) / CLOCKS_PER_SEC > timeLimit_)
                return bestActions_[s0];

            visited.clear();
            error = testConvergence(s0);
            if (error < epsilon_)
                return bestActions_[s0];
            if (error > mdplib::dead_end_cost) {
                break;  // BPSG changed, must expand tip nodes again
            }
        }
    }
}

int WeightedLAOStarSolver::expand(mlcore::State* s)
{
    if (!visited.insert(s).second)  // state was already visited
        return 0;

    int cnt = 0;

    if (s->deadEnd() || problem_->goal(s))
        return 0;

    if (bestActions_.find(s) == bestActions_.end()) {   // this means state has not been expanded
        for (mlcore::Action* a : problem_->actions()) {
            if (!problem_->applicable(s, a))
                continue;
            for (mlcore::Successor su : problem_->transition(s, a)) {
                gValues_[su.su_state] = 0.0;
                hValues_[su.su_state] = problem_->heuristic()->cost(su.su_state);
            }
        }
        cnt = 1;
    } else {
        mlcore::Action* a = bestActions_[s];
        for (mlcore::Successor sccr : problem_->transition(s, a))
            cnt += expand(sccr.su_state);
    }
    this->bellmanUpdate(s);
    return cnt;
}

double WeightedLAOStarSolver::testConvergence(mlcore::State* s)
{
    double error = 0.0;

    if (s->deadEnd() || problem_->goal(s))
        return 0.0;

    if (!visited.insert(s).second)
        return 0.0;

    mlcore::Action* prevAction = bestActions_.count(s) == 0 ? nullptr : bestActions_[s];
    if (prevAction == nullptr) {
        // hasn't converged because a state in BPSG doesn't have an action ready
        return mdplib::dead_end_cost + 1;
    } else {
        for (mlcore::Successor su : problem_->transition(s, prevAction))
            error =  std::max(error, testConvergence(su.su_state));
    }

    error = std::max(error, this->bellmanUpdate(s));
    if (prevAction == bestActions_[s])
        return error;
    return mdplib::dead_end_cost + 2; // hasn't converged because the best action changed
}

}

