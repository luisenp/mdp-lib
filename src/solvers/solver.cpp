#include <cassert>

#include "../../include/solvers/solver.h"

namespace mlsolvers
{
std::mutex bellman_mutex;

std::random_device rand_dev;

std::mt19937 gen(rand_dev());

std::uniform_real_distribution<> dis(0, 1);

#ifndef NO_META
int meta_iteration_index = -1;
mlcore::StateIntMap state_indices;
int current_state_index = 1;
std::unordered_map<int, std::list<double> > previousQValues;
#endif


double qvalue(mlcore::Problem* problem, mlcore::State* s, mlcore::Action* a)
{
    double qAction = 0.0;
    for (mlcore::Successor su : problem->transition(s, a)) {
        qAction += su.su_prob * su.su_state->cost();
    }
    qAction = (qAction * problem->gamma()) + problem->cost(s, a);
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
    double bestQ = problem->goal(s) ? 0.0 : mdplib::dead_end_cost + 1;
    bool hasAction = false;
    mlcore::Action* bestAction = nullptr;
#ifndef NO_META
    int state_idx = current_state_index;
    if (state_indices.count(s) > 0) {
        state_idx = state_indices[s];
    } else {
        state_indices[s] = state_idx;
        current_state_index += problem->actions().size();
    }
    int state_action_idx = state_idx - 1;
#endif
    for (mlcore::Action* a : problem->actions()) {
#ifndef NO_META
        state_action_idx++;
#endif
        if (!problem->applicable(s, a))
            continue;
        hasAction = true;
        double qAction = std::min(mdplib::dead_end_cost, qvalue(problem, s, a));
#ifndef NO_META
        if (previousQValues.count(state_action_idx) == 0) {
            previousQValues[state_action_idx] = std::list<double> (1, qAction);
        } else {
            std::list<double> & previousQValuesSA = previousQValues[state_action_idx];
            previousQValuesSA.push_back(qAction);
            if (previousQValuesSA.size() > 2) {
                previousQValuesSA.pop_front();
            }
        }
#endif
        if (qAction < bestQ) {
            bestQ = qAction;
            bestAction = a;
        }
    }

    if (!hasAction && bestQ >= mdplib::dead_end_cost)
        s->markDeadEnd();

    return std::make_pair(bestQ, bestAction);
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

double sampleTrial(mlcore::Problem* problem, mlcore::State* s)
{
    mlcore::State* tmp = s;
    double discount = 1.0;
    double cost = 0.0;
    while (!problem->goal(tmp)) {
        mlcore::Action* a = greedyAction(problem, tmp);
        double discountedCost = discount * problem->cost(tmp, a);
        if (discountedCost < 1.0-6)
            break;  // stop because no more cost can be accumulated (avoid infinite loop)
        cost += discountedCost;
        tmp = randomSuccessor(problem, tmp, a);
        discount *= problem->gamma();
    }
    return cost;
}

#ifndef NO_META
mlcore::Action* predictNextAction(mlcore::Problem* problem, mlcore::State* s)
{
    if (state_indices.count(s) == 0) {
        // no previous information, assume the planner will choose based on current values.
        return greedyAction(problem, s);
    }
    int state_idx = state_indices[s];
    int state_action_idx = state_idx - 1;
    double bestPredictedQValue = mdplib::dead_end_cost;
    mlcore::Action* nextAction = nullptr;
    for (mlcore::Action* a : problem->actions()) {
        state_action_idx++;
        if (!problem->applicable(s, a))
            continue;
        double predictedQValueAction = std::min(mdplib::dead_end_cost, qvalue(problem, s, a));
        // if there is not enough information, assume the QValue will remain unchanged.
        if (previousQValues.count(state_action_idx) > 0) {
            std::list<double> & previousQValuesAction = previousQValues[state_action_idx];
            if (previousQValuesAction.size() == 2) {
                predictedQValueAction +=
                    previousQValuesAction.back() - previousQValuesAction.front();
            }
        }
        if (bestPredictedQValue < bestPredictedQValue)
            nextAction = a;
    }
    return nextAction;
}
#endif

} // mlsolvers
