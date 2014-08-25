#include <queue>

#include "../../include/solvers/DeterministicSolver.h"

namespace mlsolvers
{

mlcore::Action* DeterministicSolver::solve(mlcore::State* s0)
{
    NodeComparer comp();
    std::priority_queue<Node*, std::vector<Node*>, NodeComparer> frontier(comp);
    frontier.push(new Node(nullptr, s0, nullptr, 0.0, heuristic_));
    std::list<Node*> allNodes;  // for memory clean-up later
    Node* final;
    while (!frontier.empty()) {
        Node* node = frontier.top();
        frontier.pop();
        allNodes.push_back(node);

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

            mlcore::State* nextState;
            if (choice_ == det_most_likely)
                nextState = mostLikelyOutcome(problem_, node->state(), a);

            double cost = problem_->cost(node->state(), a);

            frontier.push(new Node(node, nextState, a, cost, heuristic_, true));
        }
    }

    mlcore::Action* optimal;
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
