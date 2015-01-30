#ifndef MDPLIB_SOLVER_H
#define MDPLIB_SOLVER_H

#include <random>
#include <cassert>
#include <vector>
#include <mutex>

#include "../../include/problem.h"
#include "../../include/lexi/lexi_problem.h"
#include "../../include/lexi/lexi_state.h"
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

/**
 * Specifies whether the costs of lexiBellmanBackup should be updated according to
 * each cost function separatedly (true), or according to the chosen action (false).
 */
extern bool set_cost_v_eta;

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
 * Computes the Q-value of a state-action pair for the given lexicographical MDP.
 * The method receives the index of the value function according to which the Q-value
 * will be computed.
 *
 * This method assumes that the given action is applicable on the state.
 *
 * @param problem The problem that contains the given state.
 * @param s The state for which the Q-value will be computed.
 * @param a The action for which the Q-value will be computed.
 * @param i The index of the value function to use.
 * @return The Q-value of the state-action pair.
 */
inline double qvalue(mllexi::LexiProblem* problem, mllexi::LexiState* s, mlcore::Action* a, int i)
{
    double qAction = 0.0;
    for (mlcore::Successor su : problem->transition(s, a, 0)) {
        qAction += su.su_prob * ((mllexi::LexiState *) su.su_state)->lexiCost()[i];
    }
    qAction = (qAction * problem->gamma()) + problem->cost(s, a, i);
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
    }
    g = (g * problem->gamma()) + problem->cost(s, a);
    h *= problem->gamma();
    return std::make_pair(g, h);
}


/**
 * Performs a Bellman backup of a state.
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


/**
 * Performs a Lexicographical Bellman update of a state.
 *
 * This backup uses fSSPUDE - see http://arxiv.org/pdf/1210.4875.pdf
 *
 * @param problem The problem that contains the given state.
 * @param s The state on which the Bellman backup will be performed.
 * @return The maximum residual among all value functions.
 */
inline double lexiBellmanUpdate(mllexi::LexiProblem* problem, mllexi::LexiState* s)
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
    for (int i = 0; i < problem->size(); i++) {
        std::vector<double> qActions(filteredActions.size());
        double bestQ = mdplib::dead_end_cost + 1;
        int actionIdx = 0;
        double minCost = mdplib::dead_end_cost + 1;
        /* Computing Q-values for all actions w.r.t. the i-th cost function */
        for (mlcore::Action* a : filteredActions) {
            if (!problem->applicable(s, a))
                continue;
            qActions[actionIdx] = std::min(mdplib::dead_end_cost, qvalue(problem, s, a, i));
            if (qActions[actionIdx] < bestQ) {
                bestQ = qActions[actionIdx];
                bestAction = a;
            }
            if (problem->cost(s, a, i) < minCost) {
                minCost = problem->cost(s, a, i);
            }
//            dprint4("CHOOSE ", actionIdx, a, qActions[actionIdx]);
            actionIdx++;
        }
        if (bestQ > mdplib::dead_end_cost) {
            s->markDeadEnd();
            break;
        }

        // TODO: this is very ugly, but will do for now (Jan 2015)
        if (set_cost_v_eta) {
            double currentResidual = fabs(bestQ - s->lexiCost()[i]);
            if (currentResidual > residual)
                residual = currentResidual;
            s->setCost(bestQ, i);
        }

        /* Getting actions for the next lexicographic level */;
        std::list<mlcore::Action*> prevActions = filteredActions;
        filteredActions.clear();
        actionIdx = 0;
        for (mlcore::Action* a : prevActions) {
            if (!problem->applicable(s, a))
                continue;
            if (qActions[actionIdx] <= (bestQ * (1.0 + problem->slack()) + 1.0e-8))
            if (qActions[actionIdx] <= (bestQ + (minCost * problem->slack()) + 1.0e-8))
                filteredActions.push_back(a);

//            dprint4("FILTER ", actionIdx, a, qActions[actionIdx]);
            actionIdx++;
        }
    }

    s->setBestAction(bestAction);
    // TODO: this is very ugly, but will do for now (Jan 2015)
    if (!set_cost_v_eta) {
        for (int i = 0; i < problem->size(); i++) {
            double qChosenAction = qvalue(problem, s, bestAction, i);
            double currentResidual = fabs(qChosenAction - s->lexiCost()[i]);
            if (currentResidual > residual)
                residual = currentResidual;
            s->setCost(qChosenAction, i);
        }
    }

    return residual;
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

 * When no action is stored in the state, then action is chosen greedily on the
 * states estimated costs as stored in state.cost().
 *
 * @param problem The problem that contains the given state.
 * @param s The state for which the action will be computed.
 * @return The action with minimum Q-value.
 */
inline mlcore::Action* greedyAction(mlcore::Problem* problem, mlcore::State* s)
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
