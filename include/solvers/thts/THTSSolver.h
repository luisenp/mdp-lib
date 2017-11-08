#ifndef MDPLIB_THTSSOLVER_H
#define MDPLIB_THTSSOLVER_H

#include <vector>

#include "THTSHeuristic.h"

#include "../../Action.h"
#include "../../Problem.h"
#include "../../State.h"

#include "../Solver.h"

namespace mlsolvers
{

class ChanceNode;
class DecisionNode;
class THTSSolver;

// The possible backup functions to use.
enum THTSBackup {MONTE_CARLO = 0, MAX_MONTE_CARLO = 1, PARTIAL_BELLMAN = 2};

enum THTSOutcomeSel {TRAN_F = 0, VPI_UNIF = 1, VI_UNIF_BOUNDS = 2};

enum THTSActionSel {GREEDY = 0, UCB1 = 2};

// The possible recommendation functions to use.
enum THTSRecommendations {BEST_VALUE = 0, MOST_PLAYED = 1};

// A node in the search tree.
class THTSNode {

protected:
    // The parent node.
    THTSNode* parent_;

    // The solver to which this node belongs too.
    THTSSolver* solver_;

    // The depth at which this node is found.
    int depth_;

    // Whether this node has been solved or not.
    bool solved_;

    // The counter of how many times this node has been backed up.
    int backup_counter_;

    // The counter of how many times this node has been visited.
    int selection_counter_;

    // Initializes the node (values and counters).
    virtual void initialize(THTSSolver* solver) =0;

    // An unique index for the node.
    int index_;

    // A lower bound on the cost of the node.
    double lower_bound_;

    // An upper bound on the cost of the node.
    double upper_bound_;

    // The minimum (relative) change observed in the lower bound.
    double min_lower_bound_update_;

    // The maximum (relative) change observed in the lower bound.
    double max_lower_bound_update_;

    // The minimum (relative) change observed in the upper bound.
    double min_upper_bound_update_;

    // The maximum (relative) change observed in the lower bound.
    double max_upper_bound_update_;


public:
    THTSNode* parent() const { return parent_; }

    int depth() const { return depth_; }

    bool solved() const { return solved_; }

    int backupCounter() const { return backup_counter_; }

    int selectionCounter() const { return selection_counter_; }

    void index(int value) { index_ = value; }

    // Visits this node and performs computation on it, expanding
    // any successors if necessary.
    // The given solver is used to access information about the trials and
    // the MDP being solved.
    // The return value is the accumulated value of all nodes visited in the
    // trial, from this one on.
    virtual double visit(THTSSolver* solver) =0;

    // Performs a backup of the node given the current state of the solver,
    // and the cumulative value of the current trial starting from this node.
    virtual void backup(THTSSolver* solver, double cumulative_value) =0;

    // Prints a string representation of a node to the given stream.
    virtual std::ostream& print(std::ostream& os) const =0;

    friend std::ostream& operator<<(std::ostream& os, THTSNode* node) {
        return node->print(os);
    }
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

    // Maintains the sum of the probabilities of the solved successors.
    double total_prob_solved_successors_;

    // The set of successors that have been expanded for this node.
    std::vector<DecisionNode*> explicated_successors_;

    // Maps a state to an index in the explicated successors array.
    mlcore::StateIntMap state_successor_index_map_;

    // Updates the map from states to successor indices with the addition of
    // the given state.
    void updateSuccessorIndexMap(mlcore::State* s);

    // Returns the decision node associated with this state.
    // Input |prob| represents the probability of this decision node in the
    // successor distribution, and it's used if the decision node has to be
    // created
    DecisionNode*
    createOrGetDecisionNodeForState(mlcore::State* s, double prob);

    // Returns the decision node associated with this state only if it has
    // been stored before. Otherwise it returns a nullptr.
    DecisionNode* getDecisionNodeForStateIfPresent(mlcore::State* state);

public:
    // Creates a chance node for the given action at the given depth.
    // The counters and action value are initialized to 0.
    // The solved label is set to false.
    // The parent can't be a nullptr.
    ChanceNode(mlcore::Action* action,
               int depth,
               THTSSolver* solver,
               DecisionNode* parent);

    // Delete this node and all of its childrens.
    ~ChanceNode();

    mlcore::Action* action() const { return action_; }

    std::vector<DecisionNode*>& explicatedSuccessors() {
        return explicated_successors_;
    }

    // Overrides method in THTSNode.
    virtual double visit(THTSSolver* solver);

    // Overrides method in THTSNode.
    virtual void backup(THTSSolver* solver, double cumulative_value);

    // Overrides method in THTSNode.
    virtual void initialize(THTSSolver* solver);

    // Overrides method in THTSNode.
    virtual std::ostream& print (std::ostream& os) const {
        os << "chance-" << this->index_
            << " (" << action_ << ", " << depth_ << ")";
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

    // The cost estimate of the state represented by this node.
    double state_cost_;

    // The probability with which this node occurs in the parent's successors
    // distribution.
    double prob_;

    // Maps an action to an index in the chance node successors array.
    mlcore::ActionIntMap action_chance_node_index_map_;

    // The set of successor nodes.
    std::vector<ChanceNode*> successors_;

    // Updates the map from actions to successor chance node indices with
    // the addition of the given action.
    void updateSuccessorIndexMap(mlcore::Action* s);

    // Returns the chance node associated with this action.
    ChanceNode* getChanceNodeForAction(mlcore::Action* action);

    // Updates the parent data when this outcome is solved.
    void updateParentWithSolvedOutcome() {
        if (this->parent_ != nullptr) {
            static_cast<ChanceNode*>(this->parent_)->
                total_prob_solved_successors_ += prob_;
        }
    }

public:
    // Creates a decision node for the given state at the given depth.
    // If |parent| is nullptr, then it's expected that depth = 0 (root node).
    // The counters and action value are initialized to 0. The solved label is
    // set to false.
    // It is also possible to specify the probability of reaching the node
    // in the parent successor distribution.
    DecisionNode(mlcore::State* state,
                 int depth,
                 THTSSolver* solver,
                 ChanceNode* parent = nullptr,
                 double prob = 0.0);

    // Deletes this node and all of its children.
    ~DecisionNode();

    std::vector<ChanceNode*>& successors() { return successors_; }

    mlcore::State* state() const { return state_; }

    // Initializes the DecisionNode.
    void initialize();

    // Overrides method in THTSNode.
    virtual double visit(THTSSolver* solver);

    // Overrides method in THTSNode.
    virtual void backup(THTSSolver* solver, double cumulative_value);

    // Overrides method in THTSNode.
    virtual void initialize(THTSSolver* solver);

    // Overrides method in THTSNode.
    virtual std::ostream& print(std::ostream& os) const {
        os << "dec-" << this->index_ << " (" << state_ << ", " << depth_ << ")";
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

    // The heuristic to use for the node initialization.
    THTSHeuristic* heuristic_;

    // The root of the search tree.
    std::unique_ptr<DecisionNode> root_;

    // The backup function to use (default=MONTE_CARLO).
    THTSBackup backup_function_;

    // The recommendation function to use (default=BEST_VALUE).
    THTSRecommendations recommendation_function_;

    // The action selection rule to use.
    THTSActionSel action_selection_;

    // The outcome selection rule to use.
    THTSOutcomeSel outcome_selection_;

    // The number of trials to perform.
    int num_trials_;

    // The maximum depth for the search.
    int max_depth_;

    // The maximum number of nodes expanded per trial.
    int max_nodes_expanded_per_trial_;

    // The number of nodes expanded in the current trial.
    int num_nodes_expanded_trial_;

    // The number of virtual rollouts for initialization.
    int num_virtual_rollouts_;

    // A prior for the variance of outcomes (used for outcome selection).
    double prior_variance_outcomes_;

    // Maintains indices for the nodes created by the algorithm.
    int current_node_index_;

    // Assigns an index to the given node and updates the index counter.
    void register_node(THTSNode* node);

    // ********************************************************************** /
    //                       Outcome Selection Rules                          /
    // ********************************************************************** /
    // Returns a state randomly sampled from the normalized unsolved outcomes
    // distributions.
    mlcore::State* randomUnsolvedOutcomeSelect(ChanceNode* node, double* prob);

    // Returns a state using a value of information analysis using a uniform
    // model for the state values and the bounds updates.
    mlcore::State*
    VIUnifBoundsOutcomeSelect(ChanceNode* chance_node, double* prob);

    // Returns a state using a value of perfect information analysis,
    // using a uniform model for the state values.

    mlcore::State* VPIUnifOutcomeSelect(ChanceNode* chance_node, double* prob);

    // ********************************************************************** /
    //                        Action Selection Rules                          /
    // ********************************************************************** /
    // Computes the values of the actions for the decision node using
    // the UCB1 selection rule and stores the best ones in the given
    // vector.
    void ucb1ActionSelectRule(
        DecisionNode* node,
        double q_min,
        double q_max,
        std::vector<ChanceNode*>& best_action_nodes);

    // Stores the actions with the best Q-Value in the given vector.
    void greedyActionSelectionRule(
        DecisionNode* node, std::vector<ChanceNode*>& best_action_nodes);

public:
    THTSSolver(mlcore::Problem* problem,
               THTSHeuristic* heuristic,
               int num_trials,
               int max_depth,
               int max_nodes_expanded_per_trial,
               int num_virtual_rollouts = 0)
        : problem_(problem), heuristic_(heuristic),
          num_trials_(num_trials), max_depth_(max_depth),
          max_nodes_expanded_per_trial_(max_nodes_expanded_per_trial),
          num_virtual_rollouts_(num_virtual_rollouts) {
        num_nodes_expanded_trial_ = 0;
        root_ = nullptr;
        backup_function_ = MONTE_CARLO;
        recommendation_function_ = BEST_VALUE;
        outcome_selection_ = TRAN_F;
        prior_variance_outcomes_ = 100000.0;
        current_node_index_ = 0;
    }

    virtual ~THTSSolver() { delete_tree();}

    void backupFunction(THTSBackup value) {
        backup_function_ = value;
    }

    void actionSelection(THTSActionSel value) {
        action_selection_ = value;
    }

    void outcomeSelection(THTSOutcomeSel value) {
        outcome_selection_ = value;
    }

    void recommendationFunction(THTSRecommendations value) {
        recommendation_function_= value;
    }

    void priorVarianceOutcomes(double value) {
        prior_variance_outcomes_ = value;
    }

    // Frees the memory occupied by the search tree.
    void delete_tree() {
        current_node_index_ = 0;
        root_.reset();
    }

    // Whether or not the trial must be continued.
    bool continueTrial();

    // Selects an action for the given decision node.
    mlcore::Action* selectAction(DecisionNode* node);

    // Selects an outcome for the given chance node.
    // If |prob| != nullptr, then the method will return the probability
    // of the selected successor.
    mlcore::State* selectOutcome(ChanceNode* node, double* prob = nullptr);

    // Recommend an action for execution.
    mlcore::Action* recommend(DecisionNode* node);

    int maxDepth() const { return max_depth_; }

    // Returns the minimum possible cost that can be obtained in this node.
    double minCostNode(THTSNode* node) const { return 0.0; }

    // Returns the maximum possible cost that can be obtained in this node.
    double maxCostNode(THTSNode* node) const { return 100; }

    // Overrides method from Solver.
    virtual mlcore::Action* solve(mlcore::State* s0);
};

} // namespace mlsolvers

#endif // MDPLIB_THTSSOLVER_H
