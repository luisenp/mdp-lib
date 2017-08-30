#ifndef MDPLIB_UCTSOLVER_H
#define MDPLIB_UCTSOLVER_H

#include <unordered_map>

#include "../Action.h"
#include "../Problem.h"
#include "../State.h"

#include "Solver.h"

namespace mlsolvers
{

// A node of the UCT DAG, representing a state at a given depth.
class UCTNode
{
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

    bool operator==(const UCTNode& rhs) const
    {
        return state_ == rhs.state_ && depth_ == rhs.depth_;
    }

    // Equality function used for unordered sets/maps.
    bool equals(UCTNode *other) const
    {
        return *this == *other;
    }

    // State hash function used for unordered sets/maps.
    int hashValue() const
    {
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
 * A SSSP solver using the UCT algorithm.
 *
 * See http://link.springer.com/chapter/10.1007/11871842_29
 *
 * Implementation based on pseudocode given by
 * http://www.morganclaypool.com/doi/pdf/10.2200/S00426ED1V01Y201206AIM017
 *
 */
class UCTSolver : public Solver
{

private:
    mlcore::Problem* problem_;
    double C_;
    int maxRollouts_;
    int cutoff_;
    bool use_qvalues_for_c_;

    UCTNodeSet visited_;
    UCTNodeIntMap counters_node_;
    UCTNodeActionIntMap counters_node_action_;
    UCTNodeActionDoubleMap qvalues_;

    mlcore::Action* pickUCB1Action(UCTNode* node);
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
     * @param maxRollouts The maximum number trajectories to sample.
     * @param cutoff The maximum depth allowed for each rollout.
     * @param use_qvalues_for_c If true, the given C will be ignored and the
     *        Q-values will be used for the exploration parameter.
     */
    UCTSolver(mlcore::Problem* problem,
              int maxRollouts,
              int cutoff,
              double C = 0.0,
              bool use_qvalues_for_c = true) :
        problem_(problem), C_(C), maxRollouts_(maxRollouts),
        cutoff_(cutoff), use_qvalues_for_c_(use_qvalues_for_c) { }

    /**
     * Returns the Q-values estimated by the UCT algorithm.
     */
    UCTNodeActionDoubleMap& qvalues() { return qvalues_; }

    /**
     * Returns the counter for state-action pair visits.
     */
    UCTNodeActionIntMap& counters_node_action()
    {
        return counters_node_action_;
    }

    /**
    * Sets the maximum number of sample trajectories to gather.
    *
    * @param maxRollouts The maximum number of trajectories to sample.
    */
    void setMaxRollouts(int maxRollouts) { maxRollouts_ = maxRollouts; }

    /**
    * Sets the cutoff for the algorithm (i.e., the maximum depth of
    * each rollout).
    *
    * @param cutoff The maximum depth of the rollouts.
    */
    void setCutoff(int cutoff) { cutoff_ = cutoff; }

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
