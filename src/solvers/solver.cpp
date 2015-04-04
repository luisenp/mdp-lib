#include "../../include/solvers/solver.h"

namespace mlsolvers
{
std::mutex bellman_mutex;

std::random_device rand_dev;

std::mt19937 gen(rand_dev());

std::uniform_real_distribution<> dis(0, 1);


double qvalue(mlcore::Problem* problem, mlcore::State* s, mlcore::Action* a)
{
    double qAction = 0.0;
    for (mlcore::Successor su : problem->transition(s, a)) {
        qAction += su.su_prob * su.su_state->cost();
    }
    qAction = (qAction * problem->gamma()) + problem->cost(s, a);
    return qAction;
}


double qvalue(mllexi::LexiProblem* problem, mllexi::LexiState* s, mlcore::Action* a, int i)
{
    double qAction = 0.0;
    for (mlcore::Successor su : problem->transition(s, a, 0)) {
        qAction += su.su_prob * ((mllexi::LexiState *) su.su_state)->lexiCost()[i];
    }
    qAction = (qAction * problem->gamma()) + problem->cost(s, a, i);
    return qAction;
}


std::pair<double, double>
weightedQvalue(mlcore::Problem* problem, mlcore::State* s, mlcore::Action* a)
{
    double g = 0.0, h = 0.0;
    for (mlcore::Successor su : problem->transition(s, a)) {
        g += su.su_prob * su.su_state->gValue();
        h += su.su_prob * su.su_state->hValue();
    }
    g = (g * problem->gamma()) + problem->cost(s, a);
    h *= problem->gamma();
    return std::make_pair(g, h);
}


std::pair<double, mlcore::Action*> bellmanBackup(mlcore::Problem* problem, mlcore::State* s)
{
    double bestQ = problem->goal(s) ? 0.0 : mdplib::dead_end_cost;
    bool hasAction = false;
    mlcore::Action* bestAction = nullptr;
    for (mlcore::Action* a : problem->actions()) {
        if (!problem->applicable(s, a))
            continue;
        hasAction = true;
        double qAction = std::min(mdplib::dead_end_cost, qvalue(problem, s, a));
        if (qAction < bestQ) {
            bestQ = qAction;
            bestAction = a;
        }
    }

    if (!hasAction && bestQ == mdplib::dead_end_cost)
        s->markDeadEnd();

    return std::make_pair(bestQ, bestAction);
}


double lexiBellmanUpdate(mllexi::LexiProblem* problem, mllexi::LexiState* s, int level)
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
        double currentResidual = fabs(bestQ - s->lexiCost()[i]);
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


double bellmanUpdate(mlcore::Problem* problem, mlcore::State* s)
{
    std::pair<double, mlcore::Action*> best = bellmanBackup(problem, s);
    double residual = s->cost() - best.bb_cost;
    bellman_mutex.lock();
    s->setCost(best.bb_cost);
    s->setBestAction(best.bb_action);
    bellman_mutex.unlock();
    return fabs(residual);
}


double bellmanUpdate(mlcore::Problem* problem, mlcore::State* s, double weight)
{
    double bestQ = problem->goal(s) ? 0.0 : mdplib::dead_end_cost;
    double bestG = bestQ, bestH = bestQ;
    bool hasAction = false;
    mlcore::Action* bestAction = nullptr;
    double prevCost = s->cost();
    for (mlcore::Action* a : problem->actions()) {
        if (!problem->applicable(s, a))
            continue;
        hasAction = true;
        std::pair<double, double> gh = weightedQvalue(problem, s, a);
        double qAction = std::min(mdplib::dead_end_cost, gh.first + weight * gh.second);
        if (qAction < bestQ) {
            bestQ = qAction;
            bestG = gh.first;
            bestH = gh.second;
            bestAction = a;
        }
    }

    if (!hasAction && bestQ == mdplib::dead_end_cost)
        s->markDeadEnd();

    bellman_mutex.lock();
    s->setCost(bestQ);
    s->gValue(bestG);
    s->hValue(bestH);
    s->setBestAction(bestAction);
    bellman_mutex.unlock();

    return fabs(bestQ - prevCost);
}


mlcore::State* randomSuccessor(mlcore::Problem* problem, mlcore::State* s, mlcore::Action* a)
{
    double pick = dis(gen);

    if (a == nullptr)
        return s;

    double acc = 0.0;
    for (mlcore::Successor sccr : problem->transition(s, a)) {
        acc += sccr.su_prob;
        if (acc >= pick)
            return sccr.su_state;
    }

    return s;
}


mlcore::Action* greedyAction(mlcore::Problem* problem, mlcore::State* s)
{
    if (s->bestAction() != nullptr)
        return s->bestAction();
    mlcore::Action* bestAction;
    double bestQ = mdplib::dead_end_cost;
    bool hasAction = false;
    for (mlcore::Action* a : problem->actions()) {
        if (!problem->applicable(s, a))
            continue;
        hasAction = true;
        double qAction = std::min(mdplib::dead_end_cost, qvalue(problem, s, a));
        if (qAction < bestQ) {
            bestQ = qAction;
            bestAction = a;
        }
    }
    if (!hasAction)
        s->markDeadEnd();

    return bestAction;
}


double residual(mlcore::Problem* problem, mlcore::State* s)
{
    mlcore::Action* bestAction = greedyAction(problem, s);
    if (bestAction == nullptr)
        return 0.0; // state is a dead-end, nothing to do here
    double res = qvalue(problem, s, bestAction) - s->cost();
    return fabs(res);
}


mlcore::State* mostLikelyOutcome(mlcore::Problem* problem, mlcore::State* s, mlcore::Action* a)
{
    double prob = -1.0;
    double eps = 1.0e-6;
    std::vector<mlcore::State*> outcomes;
    for (mlcore::Successor sccr : problem->transition(s, a)) {
        if (sccr.su_prob > prob + eps) {
            prob = sccr.su_prob;
            outcomes.clear();
            outcomes.push_back(sccr.su_state);
        } else if (sccr.su_prob > prob - eps) {
            outcomes.push_back(sccr.su_state);
        }
    }
    return outcomes[rand() % outcomes.size()];
}

}
