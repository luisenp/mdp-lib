#ifndef MDPLIB_DETERMINISTICSOLVER_H
#define MDPLIB_DETERMINISTICSOLVER_H

#include "../Action.h"
#include "../State.h"

#include "Solver.h"

namespace mlsolvers
{

    const int det_most_likely = 0;

/**
 * Implements a deterministic solver based on the A* algorithm. It simplifies
 * the MDP's transition function by picking a single outcome, resulting in
 * a determinisitic problem based on the original one.
 *
 * The choice of determinisitc outcome can be specified in the constructor.
 */
class DeterministicSolver : public Solver
{
private:

    /* Indicates the choice of deterministic outcome (e.g. most-likely, least-likely) */
    int choice_;

    /* The heuristic to use */
    mlcore::Heuristic* heuristic_;

    mlcore::Problem* problem_;

public:

    DeterministicSolver() {}

    /**
     * Creates a deterministic solver with the given choice for determinization
     * and heuristic to use for A*.
     *
     * Available options the determinization are:
     *
     *      - mlsolvers::det_most_likely (Most likely outcome)
     */
    DeterministicSolver(mlcore::Problem* problem, int choice = det_most_likely,
                        mlcore::Heuristic* heuristic = nullptr)
        : problem_(problem), choice_(choice), heuristic_(heuristic) {}

    virtual ~DeterministicSolver() {}

    /**
     * Solves the associated problem using the LAO* algorithm.
     *
     * @param s0 The state to start the search at.
     */
    virtual mlcore::Action* solve(mlcore::State* s0);
};

/**
 * A search-node for A*. Includes information about a state, its parent in
 * the search graph, the action taken to reach this state, the cost to reach
 * the state from the start, and the f-value of the state (estimate of the
 * cost to reach the goal from the start going through this state).
 */
class Node
{
private:
    /* The parent node in the search graph */
    Node* parent_;

    /* The state represented by this node */
    mlcore::State* state_;

    /* The action taken by the parent state to reach this node */
    mlcore::Action* action_;

    /* The cost accumulated from the start */
    double g_;

    /* The f-value of this node (i.e., g + heuristic) */
    double f_;

public:
    Node(Node* parent,
         mlcore::State* state,
         mlcore::Action* action,
         double actionCost,
         mlcore::Heuristic* heuristic,
         bool usePathMax = true)
    {
        state_ = state;
        parent_ = parent;
        action_ = action;
        if (parent_ == nullptr) {   // assumed to be root-node
            g_ = 0.0;
            f_ = heuristic == nullptr ? 0.0 : heuristic->cost(state);
            return;
        }
        g_ = parent_->g() + actionCost;
        double h = heuristic == nullptr ? 0.0 : heuristic->cost(state);
        double hParent = heuristic == nullptr ?
                            0.0 : heuristic->cost(parent_->state());
        if (usePathMax) {
            double tmp = hParent - actionCost;
            if (tmp > h)
                f_ = g_ + tmp;
            else
                f_ = g_ + h;
        } else {
            f_ = g_ + h;
        }
    }

    mlcore::State* state() { return state_; }

    double g() { return g_; }

    double f() { return f_; }

    Node* parent() { return parent_; }

    mlcore::Action* action() { return action_; }
};

/**
 * A class for comparing two states according to their current cost.
 */
class NodeComparer
{
private:
    bool reverse_;

public:
    NodeComparer(bool reverse = false) : reverse_(reverse) { }

    bool operator() (Node* lhs, Node* rhs) const
    {
        if (reverse_)
            return (lhs->f() > rhs->f());
        else
            return (lhs->f() < rhs->f());
    }
};



}
#endif // MDPLIB_DETERMINISTICSOLVER_H
