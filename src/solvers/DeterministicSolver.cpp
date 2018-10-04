#include <queue>

#include "../../include/solvers/DeterministicSolver.h"

namespace mlsolvers
{

mlcore::Action* DeterministicSolver::solve(mlcore::State* s0)
{
    NodeComparer comp();
    std::priority_queue<Node*, std::vector<Node*>, NodeComparer> frontier(comp);
    Node* init = new Node(nullptr, s0, nullptr, 0.0, heuristic_);
    frontier.push(init);
    std::list<Node*> allNodes;  // for memory clean-up later
    allNodes.push_back(init);
    Node* final = nullptr;
    while (!frontier.empty()) {
        Node* node = frontier.top();
        frontier.pop();

        if (node->state()->checkBits(mdplib::VISITED_ASTAR))
            continue;   // valid because this is using path-max
        node->state()->setBits(mdplib::VISITED_ASTAR);

        if (problem_->goal(node->state())) {
            final = node;
            break;
        }

        for (mlcore::Action* a : problem_->actions()) {
            if (!problem_->applicable(node->state(), a))
                continue;


            double cost = problem_->cost(node->state(), a);

            mlcore::State* nextState = nullptr;
            if (choice_ == det_most_likely) {
                nextState = mostLikelyOutcome(problem_, node->state(), a);

                Node* next =
                    new Node(node, nextState, a, cost, heuristic_, true);
                frontier.push(next);
                allNodes.push_back(next);
            } else if (choice_ == det_all_outcomes) {
                for (auto& successor : problem_->transition(node->state(), a)) {
                    nextState = successor.su_state;
                    Node* next =
                        new Node(node, nextState, a, cost, heuristic_, true);
                    frontier.push(next);
                    allNodes.push_back(next);
                }
            }

        }
    }

    mlcore::Action* optimal = nullptr;
    while (final->parent() != nullptr) {
        optimal = final->action();
        final = final->parent();
    }

    for (Node* node : allNodes) {
        node->state()->clearBits(mdplib::VISITED_ASTAR);
        delete node;
    }

    return optimal;
}

}
