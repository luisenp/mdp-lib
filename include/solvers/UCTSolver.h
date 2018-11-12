#ifndef MDPLIB_UCTSOLVER_H
#define MDPLIB_UCTSOLVER_H

#include <iostream>
#include <unordered_map>

#include "../Action.h"
#include "../Problem.h"
#include "../State.h"

#include "Solver.h"

namespace mlsolvers
{

// A node of the UCT DAG, representing a state at a given depth.
class UCTNode {
public:
    // The state this node represents.
    mlcore::State* state_;

    // The depth of the state in the UCT tree.
    int depth_;

    // Creates a node for the given state at the given depth.
    UCTNode(mlcore::State* state, int depth) :
        state_(state), depth_(depth) { }

    // Destructor. Nothing to do here.
    ~UCTNode() {}

    friend std::ostream& operator<<(std::ostream& os, UCTNode* node) {
        os << "(" << node->state_ << ", " << node->depth_ << ")";
        return os;
    }

    bool operator==(const UCTNode& rhs) const {
        return state_ == rhs.state_ && depth_ == rhs.depth_;
    }

    // Equality function used for unordered sets/maps.
    bool equals(UCTNode *other) const {
        return *this == *other;
    }

    // State hash function used for unordered sets/maps.
    int hashValue() const {
        return state_->hashValue() + 31 * depth_;
    }
};

//Wrapper of the hash function for nodes (used to define the maps below).
struct UCTNodeHash {
  size_t operator()(UCTNode* node) const {
    return node->hashValue();
  }
};

//Wrapper of the equality operator for nodes (used to define the maps below).
struct UCTNodeEqual {
  bool operator() (UCTNode* node1, UCTNode* node2) const {
    return node1->equals(node2);
  }
};

// A map from nodes to to integers.
typedef std::unordered_map<UCTNode*,
                           int,
                           UCTNodeHash,
                           UCTNodeEqual> UCTNodeIntMap;

// A map from nodes to a map from action to integers.
typedef std::unordered_map<UCTNode*,
                           mlcore::ActionIntMap,
                           UCTNodeHash,
                           UCTNodeEqual> UCTNodeActionIntMap;

// A map from nodes to a map from action to doubles.
typedef std::unordered_map<UCTNode*,
                           mlcore::ActionDoubleMap,
                           UCTNodeHash,
                           UCTNodeEqual> UCTNodeActionDoubleMap;

// A set of nodes.
typedef std::unordered_set<UCTNode*, UCTNodeHash, UCTNodeEqual> UCTNodeSet;

/**
 * An SSP solver using the UCT algorithm.
 *
 * See http://link.springer.com/chapter/10.1007/11871842_29
 */
class UCTSolver : public Solver {

private:
    mlcore::Problem* problem_;

    // Exploration constant.
    double C_;

    // Maximum number of rollouts.
    int max_rollouts_;

    // Maximum depth for the rollouts.
    int cutoff_;

    // The depth used for the root node.
    int start_depth_ ;

    // If true, the start depth of the search and the cutoff are increased
    // by one after each call to |solve|.
    bool auto_adjust_depth_;

    // If true, C will always be set relative to the current Q(s,a,d).
    // The actual value will be C = Q(s,a,d) / sqrt(1 + |As|), where As
    // is the number of applicable actions for state s.
    bool use_qvalues_for_c_;

    // The number of "virtual rollouts" for Q-value initialization,
    // as used in PROST.
    int delta_;

    // Stores the nodes visited by the algorithm.
    UCTNodeSet visited_;

    // The counters for the nodes (number of times visited).
    UCTNodeIntMap counters_node_;

    // The counters for the node-action pairs.
    UCTNodeActionIntMap counters_node_action_;

    // The estimated Q-values of the actions.
    UCTNodeActionDoubleMap action_qvalues_;

    // Returns an action using the given exploration constant.
    mlcore::Action* pickAction(UCTNode* node, double C);

public:
    UCTSolver();

    virtual ~UCTSolver() {}

    /**
     * Creates an UCT solver for the given problem using the given
     * exploration parameter, maximum number of rollouts, and cutoff
     * (maximum rollout depth).
     *
     * @param problem The problem to be solved.
     * @param C The value of the exploration parameter.
     * @param max_rollouts The maximum number trajectories to sample.
     * @param cutoff The maximum depth allowed for each rollout.
     * @param use_qvalues_for_c If true, the given C will be ignored and the
     *        Q-values will be used for the exploration parameter.
     * @param delta Number of "virtual rollouts" per action for initialization.
     */
    UCTSolver(mlcore::Problem* problem,
              int max_rollouts,
              int cutoff,
              double C = 0.0,
              bool use_qvalues_for_c = true,
              int delta = 0,
              bool auto_adjust_depth = false) :
        problem_(problem), max_rollouts_(max_rollouts),
        cutoff_(cutoff), C_(C), use_qvalues_for_c_(use_qvalues_for_c),
        delta_(delta), auto_adjust_depth_(auto_adjust_depth), start_depth_(0)
    { }

    /**
     * Returns the Q-values estimated by the UCT algorithm.
     */
    UCTNodeActionDoubleMap& action_qvalues() { return action_qvalues_; }

    /**
     * Returns the counter for state-action pair visits.
     */
    UCTNodeActionIntMap& counters_node_action() {
        return counters_node_action_;
    }

    /**
     * Sets the maximum number of sample trajectories to gather.
     *
     * @param max_rollouts The maximum number of trajectories to sample.
     */
    void setmax_rollouts(int max_rollouts) { max_rollouts_ = max_rollouts; }

    /**
     * Sets the cutoff for the algorithm (i.e., the maximum depth of
     * each rollout).
     *
     * @param cutoff The maximum depth of the rollouts.
     */
    void setCutoff(int cutoff) { cutoff_ = cutoff; }

    /**
     * Sets the start depth for the search to the given value.
     */
    void setStartDepth(int depth) { start_depth_ = depth; }

    /**
     * Resets counters and resets the cutoff and start depth to original value.
     */
    virtual void reset() {
        counters_node_.clear();
        counters_node_action_.clear();
        action_qvalues_.clear();
        visited_.clear();
        cutoff_ -= start_depth_;
        start_depth_ = 0;
    }

    /**
     * Computes the UCB1 cost of the given node and action.
     *
     * @param node The node for which the cost is going to be computed.
     * @param a The action for which the cost is going to be computed.
     * @param C The value of the exploration parameter to be used.
     *
     * @return The cost of the node-action according to the UCB1 formula.
     */
    double ucb1Cost(UCTNode* node, mlcore::Action* a, double C);

    /**
     * Picks an action for the given state using the UCT algorithm.
     *
     * @param s0 The state for which the action will be chosen.
     *
     * @return The action chosen by UCT.
     */

    virtual mlcore::Action* solve(mlcore::State* s0);
};

}   // namespace mlsolvers


#endif // MDPLIB_UCTSOLVER_H
