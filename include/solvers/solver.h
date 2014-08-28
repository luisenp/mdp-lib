#ifndef MDPLIB_SOLVER_H
#define MDPLIB_SOLVER_H

#include <random>
#include <cassert>
#include <vector>
#include <mutex>

#include "../../include/problem.h"
#include "../../include/heuristic.h"
#include "../../include/state.h"
#include "../../include/util/general.h"

#define bb_cost first
#define bb_action second

namespace mlsolvers
{
/**
 * A mutex used by BellmanUpdate in order to avoid race conditions while
 * planning/executing concurrently.
 */
extern std::mutex bellman_mutex;

/**
 * Random number generator.
 */
extern std::random_device rand_dev;

/**
 * Mersenne Twister 19937 generator.
 */
extern std::mt19937 gen;

/**
 * Uniform distribution [0,1] generator.
 */
extern std::uniform_real_distribution<> dis;

extern int bbcount;

/**
 * An interface for states to have some polymorphism on methods that want to call
 * different planners.
 */
class Solver
{
public:

    /**
     * Solves the associated problem using this solver.
     *
     * @param s0 The state to start the search at.
     * @return The action recommended by this algorithm for state s0.
     */
    virtual mlcore::Action* solve(mlcore::State* s0) =0;
};

/**
 * Computes the Q-value of a state-action pair.
 * This method assumes that the given action is applicable on the state.
 *
 * @param problem The problem that contains the given state.
 * @param s The state for which the Q-value will be computed.
 * @param a The action for which the Q-value will be computed.
 * @return The Q-value of the state-action pair.
 */
inline double qvalue(mlcore::Problem* problem, mlcore::State* s, mlcore::Action* a)
{
    double qAction = 0.0;
    for (mlcore::Successor su : problem->transition(s, a)) {
        qAction += su.su_prob * su.su_state->cost();
    }
    qAction = (qAction * problem->gamma()) + problem->cost(s, a);
    return qAction;
}

/**
 * Computes the weighted-Q-value of a state-action pair.
 * This method assumes that the given action is applicable on the state.
 *
 * @param problem The problem that contains the given state.
 * @param s The state for which the Q-value will be computed.
 * @param a The action for which the Q-value will be computed.
 * @param weight The weight to use.
 * @return A pair of doubles representing the g-value and h-value of the state-action
 *        pair. The weighted-Q-value can be recovered as g + weight * h.
 */
inline std::pair<double, double>
weightedQvalue(mlcore::Problem* problem, mlcore::State* s, mlcore::Action* a)
{
    double g = 0.0, h = 0.0;
    for (mlcore::Successor su : problem->transition(s, a)) {
        g += su.su_prob * su.su_state->gValue();
        h += su.su_prob * su.su_state->hValue();
        dprint3("*********", su.su_state, su.su_state->hValue());
    }
    g = (g * problem->gamma()) + problem->cost(s, a);
    h *= problem->gamma();
    dprint3(s, a, h);
    return std::make_pair(g, h);
}


/**
 * Performs a Bellman backup a state.
 *
 * This backup uses fSSPUDE - see http://arxiv.org/pdf/1210.4875.pdf
 *
 * @param problem The problem that contains the given state.
 * @param s The state on which the Bellman backup will be performed.
 * @return A pair containing the estimated cost and estimated best action according
 *        to this Bellman backup.
 */
inline
std::pair<double, mlcore::Action*> bellmanBackup(mlcore::Problem* problem, mlcore::State* s)
{
    bbcount++;
    double bestQ = problem->goal(s) ? 0.0 : mdplib::dead_end_cost + 1;
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

/**
 * Performs a Bellman backup of a state, and then updates the state with
 * the resulting expected cost and greedy action.
 *
 * @param problem The problem that contains the given state.
 * @param s The state on which the Bellman backup will be performed.
 * @return The residual of the state.
 */
inline double bellmanUpdate(mlcore::Problem* problem, mlcore::State* s)
{
    std::pair<double, mlcore::Action*> best = bellmanBackup(problem, s);
    double residual = s->cost() - best.bb_cost;
    bellman_mutex.lock();
    s->setCost(best.bb_cost);
    s->setBestAction(best.bb_action);
    bellman_mutex.unlock();
    return fabs(residual);
}

/**
 * Performs a weighted-Bellman backup a state, and then updates the state with
 * the resulting expected cost and greedy action.
 *
 * This backup uses fSSPUDE - see http://arxiv.org/pdf/1210.4875.pdf
 *
 * @param problem The problem that contains the given state.
 * @param s The state on which the Bellman backup will be performed.
 * @param weight The weight to use.
 * @return The residual of the state.
 */
inline double bellmanUpdate(mlcore::Problem* problem, mlcore::State* s, double weight)
{
    if (weight == 1.0)
        return bellmanUpdate(problem, s);

    double bestQ = problem->goal(s) ? 0.0 : mdplib::dead_end_cost + 1;
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

/**
 * Samples a successor state of a state and action using the probabilities
 * defined by the problem's transition function.
 *
 * If the given action is a null pointer or if the state is a dead-end (i.e., the
 * transition function returns empty list of successors) this method will return
 * the same state that is given.
 *
 * @param problem The problem that defines the transition function.
 * @param s The state for which the sucessor state will be sampled.
 * @param a The action that generates the successors.
 * @return A successor sampled from the transition function corresponding to the
 *        state and action pair.
 */
inline mlcore::State*
            randomSuccessor(mlcore::Problem* problem, mlcore::State* s, mlcore::Action* a)
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

/**
 * Returns the action with minimum Q-value for a state.
 *
 * This method assumes that any action stored state.bestAction() is consistent with
 * the latest expected costs in the problem. This is guaranteed by any solver that
 * performs backup operations through calls to bellmanUpdate(problem, state).

 * When no action is stored in the state, then a Bellman backup is performed to
 * compute the action with minimum Q-value.
 *
 * @param problem The problem that contains the given state.
 * @param s The state for which the action will be computed.
 * @return The action with minimum Q-value.
 */
inline mlcore::Action* greedyAction(mlcore::Problem* problem, mlcore::State* s)
{
    if (s->bestAction() == nullptr)
        bellmanUpdate(problem, s);
    return s->bestAction();
}

/**
 * Computes the residual of a state.
 *
 * @param problem The problem that contains the given state.
 * @param s The state for which the residual will be computed.
 * @return The residual of the given state.
 */
inline double residual(mlcore::Problem* problem, mlcore::State* s)
{
    mlcore::Action* bestAction = greedyAction(problem, s);
    if (bestAction == nullptr)
        return 0.0; // state is a dead-end, nothing to do here
    double res = qvalue(problem, s, bestAction) - s->cost();
    return fabs(res);
}

/**
 * Returns the most likely outcome after executing the given action on the given
 * state, according to the transition function of the given problem.
 *
 * @param problem The problem that defines the transition function.
 * @param s The state for which the most likely outcome will be obtained.
 * @param a The action executed in the given state.
 * @return The most likely outcome of the state and action pair.
 */
inline mlcore::State* mostLikelyOutcome(mlcore::Problem* problem,
                                         mlcore::State* s, mlcore::Action* a)
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


#endif // MDPLIB_SOLVER_H
