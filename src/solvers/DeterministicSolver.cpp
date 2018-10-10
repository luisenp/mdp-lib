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
    Node* finalNode = nullptr;
                                                                                dprint("a");
    while (!frontier.empty()) {
                                                                                dprint("b");
        Node* node = frontier.top();
        frontier.pop();

                                                                                dprint("c");
        if (node->state()->checkBits(mdplib::VISITED_ASTAR))
            continue;   // valid because this is using path-max
        node->state()->setBits(mdplib::VISITED_ASTAR);
                                                                                dprint("d");

        if (problem_->goal(node->state())) {
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
                                                                                if (finalNode->state() == nullptr)
                                                                                    dprint("wtf");
    costLastPathFound_ = finalNode->g();
    while (finalNode->parent() != nullptr) {
        optimal = finalNode->action();
        finalNode = finalNode->parent();
    }

    for (Node* node : allNodes) {
        node->state()->clearBits(mdplib::VISITED_ASTAR);
        delete node;
    }

    return optimal;
}

}
