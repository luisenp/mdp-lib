#ifndef MDPLIB_THTSSOLVER_H
#define MDPLIB_THTSSOLVER_H

#include <vector>

#include "../Action.h"
#include "../Problem.h"
#include "../State.h"

#include "Solver.h"

namespace mlsolvers
{

class ChanceNode;
class DecisionNode;
class THTSSolver;

// A node in the search tree.
class THTSNode {

protected:
    // The parent node.
    THTSNode* parent_;

    // The depth at which this node is found.
    int depth_;

    // Whether this node has been solved or not.
    bool solved_;

    // The counter of how many times this node has been backed up.
    int backup_counter_;

    // The counter of how many times this node has been visited.
    int selection_counter_;

    void initialize() {
        this->backup_counter_ = 0;
        this->selection_counter_ = 0;
        this->solved_ = false;
    }

public:
    THTSNode* parent() const { return parent_; }

    int depth() const { return depth_; }

    bool solved() const { return solved_; }

    int backupCounter() const { return backup_counter_; }

    int selectionCounter() const { return selection_counter_; }

    void increaseBackupCounter() { this->backup_counter_++; }

    void increaseSelectionCounter() { this->selection_counter_++; }

    // Visits this node and performs computation on it, expanding
    // any successors if necessary.
    // The given problem is used to access the transition and reward
    // functions of the MDP.
    // The given solver is used to access information about the trials.
    // The return value is the accumulated value of all nodes visited in the
    // trial, from this one on.
    virtual double visit(THTSSolver* solver, mlcore::Problem* problem) =0;

    // Performs a backup of the node given the current state of the solver,
    // and the cumulative value of the current trial starting from this node.
    virtual void backup(THTSSolver* solver, double cumulative_value) =0;
};

// A chance node in the search tree, representing a state-action pair.
class ChanceNode : public THTSNode {

friend class DecisionNode;
friend class THTSSolver;

private:
    // The action that this node corresponds to.
    mlcore::Action* action_;

    // The value estimate of the state-action pair represented by this node.
    double action_value_;

    // The set of successors that have been expanded for this node.
    std::vector<DecisionNode*> explicated_successors_;

    // Maps a state to an index in the explicated successors array.
    mlcore::StateIntMap state_successor_index_map_;

    // Updates the map from states to successor indices with the addition of
    // the given state.
    void updateSuccessorIndexMap(mlcore::State* s);

    // Returns the decision node associated with this state.
    DecisionNode* getDecisionNodeForState(mlcore::State* s);

public:
    // Creates a chance node for the given action at the given depth.
    // The counters and action value are initialized to 0.
    // The solved label is set to false.
    // The parent can't be a nullptr.
    ChanceNode(mlcore::Action* action, int depth, DecisionNode* parent);

    mlcore::Action* action() const { return action_; }

    std::vector<DecisionNode*>& explicatedSuccessors() {
        return explicated_successors_;
    }

    // Overrides method in THTSNode.
    virtual double visit(THTSSolver* solver, mlcore::Problem* problem);

    // Overrides method in THTSNode.
    virtual void backup(THTSSolver* solver, double cumulative_value);

    friend std::ostream& operator<<(std::ostream& os, ChanceNode* node) {
        os << "chance (" << node->action_ << ", " << node->depth_ << ")";
        return os;
    }

};

// A decision node in the search tree, representing a state.
class DecisionNode : public THTSNode {

friend class ChanceNode;
friend class THTSSolver;

private:
    // The state that this node corresponds to.
    mlcore::State* state_;

    // The value estimate of the state represented by this node.
    double state_value_;

    // Maps an action to an index in the chance node successors array.
    mlcore::ActionIntMap action_chance_node_index_map_;

    // The set of successor nodes.
    std::vector<ChanceNode*> successors_;

    // Updates the map from actions to successor chance node indices with
    // the addition of the given action.
    void updateSuccessorIndexMap(mlcore::Action* s);

    // Returns the chance node associated with this action.
    ChanceNode* getChanceNodeForAction(mlcore::Action* action);

public:
    // Creates a decision node for the given state at the given depth.
    // If |parent| is nullptr, then it's expected that depth = 0 (root node).
    // The counters and action value are initialized to 0. The solved label is
    // set to false.
    DecisionNode(mlcore::State* state, int depth, ChanceNode* parent = nullptr);

    std::vector<ChanceNode*>& successors() { return successors_; }

    mlcore::State* state() const { return state_; }

    // Overrides method in THTSNode.
    virtual double visit(THTSSolver* solver, mlcore::Problem* problem);

    // Overrides method in THTSNode.
    virtual void backup(THTSSolver* solver, double cumulative_value);

    friend std::ostream& operator<<(std::ostream& os, DecisionNode* node) {
        os << "dec (" << node->state_ << ", " << node->depth_ << ")";
        return os;
    }
};


// A Trial-based Heuristic Tree Search solver.
// See http://ai.cs.unibas.ch/papers/keller-dissertation.pdf.
class THTSSolver : public Solver {

friend class DecisionNode;
friend class ChanceNode;

private:
    // The problem describing the MDP to solve.
    mlcore::Problem* problem_;

    // The number of trials to perform.
    int num_trials_;

    // The maximum depth for the search.
    int max_depth_;

    // The maximum number of nodes expanded per trial.
    int max_nodes_expanded_per_trial_;

    // Maintains the number of nodes expanded in the current trial.
    int num_nodes_expanded_trial_;

public:
    THTSSolver(mlcore::Problem* problem,
               int num_trials,
               int max_depth,
               int max_nodes_expanded_per_trial) :
        problem_(problem), num_trials_(num_trials), max_depth_(max_depth),
        max_nodes_expanded_per_trial_(max_nodes_expanded_per_trial) {
        num_nodes_expanded_trial_ = 0;
    }

    virtual ~THTSSolver() {}

    // Whether or not the trial must be continued.
    bool continueTrial();

    // Selects an action for the given decision node.
    mlcore::Action* selectAction(DecisionNode* node);

    // Selects an outcome for the given chance node.
    mlcore::State* selectOutcome(ChanceNode* node);

    int maxDepth() const { return max_depth_; }

    // Overrides method from Solver.
    virtual mlcore::Action* solve(mlcore::State* s0);
};

} // namespace mlsolvers

#endif // MDPLIB_THTSSOLVER_H
