#include <queue>

#include "../../include/solvers/DeterministicSolver.h"

namespace mlsolvers
{

mlcore::Action* DeterministicSolver::solve(mlcore::State* s0)
{
    NodeComparer comp();
    std::priority_queue<Node*, std::vector<Node*>, NodeComparer> frontier(comp);
    Node* init = new Node(nullptr, s0, nullptr, 0.0, heuristic_, cache_);
    frontier.push(init);
    std::list<Node*> allNodes;  // for memory clean-up later
    allNodes.push_back(init);
    Node* finalNode = nullptr;
    while (!frontier.empty()) {
        Node* node = frontier.top();
        frontier.pop();
        if (node->state()->checkBits(mdplib::VISITED_ASTAR))
            continue;   // valid because this is using path-max
        node->state()->setBits(mdplib::VISITED_ASTAR);
        if (problem_->goal(node->state()) || cache_.count(node->state())) {
//                                                                                if (cache_.count(node->state())) {
//                                                                                    dprint("cache-hit", node->state(), cache_.at(node->state()),
//                                                                                           node->g(), node->f());
//                                                                                }
            finalNode = node;
            break;
        }

        for (mlcore::Action* a : problem_->actions()) {
            if (!problem_->applicable(node->state(), a))
                continue;


            double cost = problem_->cost(node->state(), a);

            mlcore::State* nextState = nullptr;
            if (choice_ == det_most_likely || choice_ == det_random) {
                if (choice_ == det_most_likely)
                    nextState = mostLikelyOutcome(problem_, node->state(), a);
                else
                    nextState = randomSuccessor(problem_, node->state(), a);

                Node* next = new Node(
                    node, nextState, a, cost, heuristic_, cache_, true);
                frontier.push(next);
                allNodes.push_back(next);
            } else if (choice_ == det_all_outcomes) {
                for (auto& successor : problem_->transition(node->state(), a)) {
                    nextState = successor.su_state;
                    Node* next = new Node(
                        node, nextState, a, cost, heuristic_, cache_, true);
                    frontier.push(next);
                    allNodes.push_back(next);
                }
            }

        }
    }

    mlcore::Action* optimal = nullptr;
    double costToGo = 0.0;
    costLastPathFound_.clear();
    while (finalNode->parent() != nullptr) {
        optimal = finalNode->action();
        mlcore::State* s = finalNode->state();
        if (cache_.count(s))
            costToGo += cache_.at(s);
        else if (!problem_->goal(s))
            costToGo += problem_->cost(s, optimal);
        costLastPathFound_[s] = costToGo;
        cache_[s] = costToGo;
        finalNode = finalNode->parent();
    }
    optimal = finalNode->action();
    costToGo += problem_->cost(finalNode->state(), optimal);
    costLastPathFound_[finalNode->state()] = costToGo;
    cache_[finalNode->state()] = costToGo;

    for (Node* node : allNodes) {
        node->state()->clearBits(mdplib::VISITED_ASTAR);
        delete node;
    }

    return optimal;
}

mlcore::Action* DeterministicSolver::solveTree(mlcore::State* s0, int horizon) {
    using namespace std;
    using namespace mlcore;

    NodeComparer comp();
    priority_queue<Node*, vector<Node*>, NodeComparer> frontier(comp);
    Node* init = new Node(nullptr, s0, nullptr, 0.0, heuristic_, cache_);
    frontier.push(init);
    list<Node*> allNodes;  // for memory clean-up later
    allNodes.push_back(init);
    Node* finalNode = nullptr;
    double besTotalCost = mdplib::dead_end_cost + 1;
                                                                                int cnt = 0;
                                                                                dprint("------1");
    vector< StateDoubleMap > bestCostsPerDepth(horizon + 1, StateDoubleMap());
    bestCostsPerDepth[0][s0] = 0;
    while (!frontier.empty()) {
        Node* node = frontier.top();
        frontier.pop();
                                                                                cnt++;
                                                                                dprint(node->state(), node->depth(), frontier.size());

        if (problem_->goal(node->state())) {
            finalNode = node;
            break;
        }
//                                                                                dprint("--1.1");


        if (node->depth() == horizon) {
            if (node->f() < besTotalCost) {
                finalNode = node;
                besTotalCost = node->f();
            }
        } else if (node->depth() > horizon) {
            continue;
        }

        for (mlcore::Action* a : problem_->actions()) {
            if (!problem_->applicable(node->state(), a))
                continue;
//                                                                                dprint("-------", a);


            double cost = problem_->cost(node->state(), a);

            mlcore::State* nextState = nullptr;
            if (choice_ == det_most_likely || choice_ == det_random) {
                if (choice_ == det_most_likely)
                    nextState = mostLikelyOutcome(problem_, node->state(), a);
                else
                    nextState = randomSuccessor(problem_, node->state(), a);

                Node* next = new Node(
                    node, nextState, a, cost, heuristic_, cache_, true);
                int d = next->depth();
                if (!bestCostsPerDepth[d].count(nextState)
                        || bestCostsPerDepth[d][nextState] > next->g()) {
                    bestCostsPerDepth[d][nextState] = next->g();
                    frontier.push(next);
                    allNodes.push_back(next);
                }

//                                                                                dprint("----------", nextState);
            } else if (choice_ == det_all_outcomes) {
                for (auto& successor : problem_->transition(node->state(), a)) {
                    nextState = successor.su_state;
                    Node* next = new Node(
                        node, nextState, a, cost, heuristic_, cache_, true);

                    int d = next->depth();
                    if (!bestCostsPerDepth[d].count(nextState)
                            || bestCostsPerDepth[d][nextState] > next->g()) {
                        bestCostsPerDepth[d][nextState] = next->g();
                        frontier.push(next);
                        allNodes.push_back(next);
                    }
                }
            }

        }
    }
                                                                                exit(0);
                                                                                dprint("------2", cnt);

    mlcore::Action* optimal = nullptr;
    double costToGo = 0.0;
    costLastPathFound_.clear();
    while (finalNode->parent() != nullptr) {
        optimal = finalNode->action();
        if (!problem_->goal(finalNode->state()))
            costToGo += problem_->cost(finalNode->state(), optimal);
        costLastPathFound_[finalNode->state()] = costToGo;
        finalNode = finalNode->parent();
    }
    optimal = finalNode->action();
    costToGo += problem_->cost(finalNode->state(), optimal);
    costLastPathFound_[finalNode->state()] = costToGo;

    return optimal;
}

}
