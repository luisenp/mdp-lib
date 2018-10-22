#include <cassert>
#include <list>

#include "../../include/solvers/Solver.h"

namespace mlsolvers
{
std::mutex bellman_mutex;

std::random_device rand_dev;

std::mt19937 kRNG(1234);

std::uniform_real_distribution<> kUnif_0_1(0, 1);


double qvalue(mlcore::Problem* problem, mlcore::State* s, mlcore::Action* a)
{
    double qAction = 0.0;
    for (const mlcore::Successor& su : problem->transition(s, a)) {
        qAction += su.su_prob * su.su_state->cost();
    }
    qAction = (qAction * problem->gamma()) + problem->cost(s, a);
    return qAction;
}

std::pair<double, double>
weightedQvalue(mlcore::Problem* problem, mlcore::State* s, mlcore::Action* a)
{
    double g = 0.0, h = 0.0;
    for (const mlcore::Successor& su : problem->transition(s, a)) {
        g += su.su_prob * su.su_state->gValue();
        h += su.su_prob * su.su_state->hValue();
    }
    g = (g * problem->gamma()) + problem->cost(s, a);
    h *= problem->gamma();
    return std::make_pair(g, h);
}


std::pair<double, mlcore::Action*> bellmanBackup(mlcore::Problem* problem,
                                                 mlcore::State* s)
{
    double bestQ = problem->goal(s) ? 0.0 : mdplib::dead_end_cost;
    bool hasAction = false;
    mlcore::Action* bestAction = nullptr;
    for (mlcore::Action* a : problem->actions()) {
        if (!problem->applicable(s, a))
            continue;
        hasAction = true;
        double qAction = std::min(mdplib::dead_end_cost, qvalue(problem, s, a));
        if (qAction <= bestQ) {
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
    if (weight == 1.0)
        return bellmanUpdate(problem, s);
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
        double qAction = std::min(mdplib::dead_end_cost,
                                   gh.first + weight * gh.second);
        if (qAction <= bestQ) {
            bestQ = qAction;
            bestG = gh.first;
            bestH = gh.second;
            bestAction = a;
        }
    }

    if (!hasAction && bestQ == mdplib::dead_end_cost)
        s->markDeadEnd();

    bestG = std::min(bestG, mdplib::dead_end_cost);
    bestH = std::min(bestH, mdplib::dead_end_cost);
    bellman_mutex.lock();
    s->setCost(bestQ);
    s->gValue(bestG);
    s->hValue(bestH);
    s->setBestAction(bestAction);
    bellman_mutex.unlock();

    return fabs(bestQ - prevCost);
}


mlcore::State* randomSuccessor(mlcore::Problem* problem,
                               mlcore::State* s,
                               mlcore::Action* a,
                               double* prob)
{
    double pick = kUnif_0_1(kRNG);

    if (a == nullptr)
        return s;

    double acc = 0.0;
    for (mlcore::Successor sccr : problem->transition(s, a)) {
        acc += sccr.su_prob;
        if (acc >= pick) {
            if (prob != nullptr)
                *prob = sccr.su_prob;
            return sccr.su_state;
        }
    }
    if (prob != nullptr)
        *prob = 1.0;
    return s;
}


mlcore::Action* greedyAction(mlcore::Problem* problem, mlcore::State* s)
{
    if (s->bestAction() != nullptr)
        return s->bestAction();
    mlcore::Action* bestAction = nullptr;
    double bestQ = mdplib::dead_end_cost;
    bool hasAction = false;
    for (mlcore::Action* a : problem->actions()) {
        if (!problem->applicable(s, a))
            continue;
        hasAction = true;
        double qAction = std::min(mdplib::dead_end_cost, qvalue(problem, s, a));
        if (qAction <= bestQ) {
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


mlcore::State* mostLikelyOutcome(mlcore::Problem* problem, mlcore::State* s,
                                 mlcore::Action* a, bool noTies)
{
    double prob = -1.0;
    double eps = 1.0e-6;
    std::vector<mlcore::State*> outcomes;
    for (mlcore::Successor sccr : problem->transition(s, a)) {
        if (sccr.su_prob > prob + eps) {
            prob = sccr.su_prob;
            outcomes.clear();
            outcomes.push_back(sccr.su_state);
        } else if (sccr.su_prob >= prob - eps) {
            outcomes.push_back(sccr.su_state);
        }
    }
    if (noTies)
        return outcomes[0];
    return outcomes[0];
    //TODO: move it back to this version
//    return outcomes[rand() % outcomes.size()];
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
            break;  // stop  to avoid infinite loop
        cost += discountedCost;
        tmp = randomSuccessor(problem, tmp, a);
        discount *= problem->gamma();
    }
    return cost;
}


bool getReachableStates(mlcore::Problem* problem,
                        mlcore::StateSet& reachableStates,
                        mlcore::StateSet& tipStates,
                        int horizon)
{
    bool containsGoal = false;
    std::list< std::pair<mlcore::State *, int> > stateDepthQueue;
    if (reachableStates.empty()) {
        stateDepthQueue.push_front(std::make_pair(problem->initialState(), 0));
        reachableStates.insert(problem->initialState());
    } else {
        for (auto const & state : reachableStates)
            stateDepthQueue.push_front(std::make_pair(state, 0));
    }
    bool goalSeen = false;
    tipStates.clear();
    while (!stateDepthQueue.empty()) {
        auto stateDepthPair = stateDepthQueue.back();
        stateDepthQueue.pop_back();
        mlcore::State* state = stateDepthPair.first;
        int depth = stateDepthPair.second;
        if (problem->goal(state)) {
            tipStates.insert(state);
            containsGoal = true;
            continue;
        }
        if (depth == horizon) {
            tipStates.insert(state);
            continue;
        }
        for (mlcore::Action* a : problem->actions()) {
            if (!problem->applicable(state, a))
                continue;
            for (mlcore::Successor sccr : problem->transition(state, a)) {
                if (reachableStates.insert(sccr.su_state).second)
                    stateDepthQueue.
                        push_front(std::make_pair(sccr.su_state, depth + 1));
            }
        }
    }
    return goalSeen;
}


bool getReachableStatesTrajectoryProbs(mlcore::Problem* problem,
                                       mlcore::State* s,
                                       mlcore::StateSet& reachableStates,
                                       mlcore::StateSet& tipStates,
                                       double rho)
{
    bool containsGoal = false;
    std::list< std::pair<mlcore::State *, double> > trajProbQueue;
    trajProbQueue.push_front(std::make_pair(s, 0.0));
    bool goalSeen = false;
    reachableStates.clear();
    tipStates.clear();
    double log_rho = -std::log(rho);
    reachableStates.insert(s);
    while (!trajProbQueue.empty()) {
        auto stateDepthPair = trajProbQueue.back();
        trajProbQueue.pop_back();
        mlcore::State* state = stateDepthPair.first;
        double depth = stateDepthPair.second;
        if (problem->goal(state)) {
            tipStates.insert(state);
            containsGoal = true;
            continue;
        }
        if (depth > log_rho) {
            tipStates.insert(state);
            continue;
        }
        for (mlcore::Action* a : problem->actions()) {
            if (!problem->applicable(state, a))
                continue;
            for (mlcore::Successor sccr : problem->transition(state, a)) {
                double newDepth = depth - std::log(sccr.su_prob);
                if (reachableStates.insert(sccr.su_state).second) {
                    trajProbQueue.
                        push_front(std::make_pair(sccr.su_state, newDepth));
                }
            }
        }
    }
    return goalSeen;
}


void getBestPartialSolutionGraph(mlcore::Problem* problem,
                                 mlcore::State* initialState,
                                 mlcore::StateSet& bpsg)
{
    std::list<mlcore::State *> stateStack;
    stateStack.push_front(initialState);
    while (!stateStack.empty()) {
        mlcore::State* state = stateStack.front();
        stateStack.pop_front();
        if (!bpsg.insert(state).second)
            continue;
        if (problem->goal(state))
            continue;
        mlcore::Action* a = greedyAction(problem, state);
        for (mlcore::Successor sccr : problem->transition(state, a)) {
            stateStack.push_front(sccr.su_state);
        }
    }
}

// Code based on Tarjan's strongly connected component algorithm
// See https://en.wikipedia.org/wiki/Tarjan%27s_strongly_connected_components_algorithm
bool testDeadEndRecursion(mlcore::Problem* problem,
                          mlcore::State* state,
                          mlcore::StateIntMap& indices,
                          mlcore::StateIntMap& lowLinks,
                          mlcore::StateSet& onStack,
                          int& index,
                          bool& goal_reached)
{
    indices[state] = index;
    lowLinks[state] = index;
    index++;
    onStack.insert(state);

    // Adding all possible successors of this state. If the state is a goal
    // then we add all possible states. This is done to make sure that all
    // states that can reach the goal are in the same strongly connected
    // component as the goal
    std::list<mlcore::State*> successors;
    if (problem->goal(state)) {
        goal_reached = true;
        for (mlcore::State* next : problem->states()) {
            successors.push_back(next);
        }
    } else {
        for (mlcore::Action* a : problem->actions()) {
            if (!problem->applicable(state, a))
                continue;
            for (auto & successor: problem->transition(state, a)) {

                mlcore::State* next = successor.su_state;
                successors.push_back(next);
            }
        }
    }

    // Now recursing on successor states to see which component they belong to
    bool no_dead_end = true;
    for (mlcore::State* next : successors) {
        if (indices.count(next) == 0) {
            no_dead_end &= testDeadEndRecursion(
                problem, next, indices, lowLinks, onStack, index, goal_reached);
            lowLinks[state] = std::min(lowLinks.at(state), lowLinks.at(next));
        } else if (onStack.count(next)) {
            lowLinks[state] = std::min(lowLinks.at(state), indices.at(next));
        }
    }

    if (lowLinks.at(state) == indices.at(state)) {
        if (lowLinks.at(state) != 0)
            return false;
    }
    return no_dead_end;
}

bool testDeadEnds(mlcore::Problem* problem)
{
    problem->generateAll();
    int index = 0;
    mlcore::StateIntMap indices;
    mlcore::StateIntMap lowLinks;
    mlcore::StateSet onStack;
    bool goal_reached = false;
    bool singleComponent = testDeadEndRecursion(
        problem, problem->initialState(),
        indices, lowLinks, onStack, index, goal_reached);
    return singleComponent && goal_reached;
}


} // mlsolvers
