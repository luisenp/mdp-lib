#include "../../include/solvers/THTSSolver.h"

#include <limits>

using namespace std;

namespace mlsolvers
{
////////////////////////////
// ChanceNode
///////////////////////////
ChanceNode::ChanceNode(mlcore::Action* action,
                       int depth,
                       DecisionNode* parent) {
    assert(parent != nullptr);
    this->parent_ = parent;
    this->depth_ = depth;
    this->initialize();
    action_ = action;
    action_value_ = 0.0;
}

void ChanceNode::backup(THTSSolver* solver) {
    this->increaseBackupCounter();
    if (this->depth_ == solver->max_depth_) {
        action_value_ =
            static_cast<DecisionNode*>(this->parent_)->state_->cost();
    } else {
        //TODO: Complete

    }
}

void ChanceNode::updateSuccessorIndexMap(mlcore::State* state) {
    state_successor_index_map_[state] = explicated_successors_.size() - 1;
}

DecisionNode* ChanceNode::getDecisionNodeForState(mlcore::State* state) {
    if (state_successor_index_map_.count(state)) {
        // Successor state has already been explicated.
        int successor_node_index = state_successor_index_map_[state];
        assert(successor_node_index < explicated_successors_.size());
        return explicated_successors_[successor_node_index];
    } else {
        // Successor state has never been explicated, add a new node.
        explicated_successors_.push_back(
            new DecisionNode(state, this->depth_ + 1, this));
        updateSuccessorIndexMap(state);
    }
}

void ChanceNode::visit(THTSSolver* solver, mlcore::Problem* problem) {
    this->increaseSelectionCounter();
    if (solver->continueTrial() && this->depth_ < solver->max_depth_) {
        mlcore::State* s = solver->selectOutcome(this);
        DecisionNode* node = getDecisionNodeForState(s);
        node->visit(solver, problem);
        this->backup(solver);
    }
}

////////////////////////////
// DecisionNode
///////////////////////////
DecisionNode::DecisionNode(mlcore::State* state,
                           int depth,
                           ChanceNode* parent) {
    if (parent == nullptr)
        assert(depth = 0);
    this->parent_ = parent;
    this->depth_ = depth;
    this->initialize();
    state_ = state;
    state_value_ = 0.0;
}

ChanceNode* DecisionNode::getChanceNodeForAction(mlcore::Action* action) {
    // For decision nodes, all successor chance nodes must be explicated.
    assert(action_chance_node_index_map_.count(action));
    int chance_node_index = action_chance_node_index_map_[action];
    assert(chance_node_index < successors_.size());
    return successors_[chance_node_index];
}

void DecisionNode::updateSuccessorIndexMap(mlcore::Action* action) {
    action_chance_node_index_map_[action] = successors_.size() - 1;
}

void DecisionNode::backup(THTSSolver* solver) {
    this->increaseBackupCounter();
    state_value_ = std::numeric_limits<double>::max();
    for (ChanceNode* successor : successors_) {
        if (successor->action_value_ < state_value_) {
            state_value_ = successor->action_value_;
        }
    }
}

void DecisionNode::visit(THTSSolver* solver, mlcore::Problem* problem) {
    if (successors_.empty()) {   // not expanded yet
        for (mlcore::Action* action : problem->actions()) {
            successors_.push_back(new ChanceNode(action, this->depth_, this));
            updateSuccessorIndexMap(action);
            // TODO: initialize the decision node
        }
    }
    this->increaseSelectionCounter();
    mlcore::Action* a = solver->selectAction(this);
    ChanceNode* chance_node = getChanceNodeForAction(a);
    chance_node->visit(solver, problem);
    this->backup(solver);
}

////////////////////////////
// THTSSolver
///////////////////////////
mlcore::Action* THTSSolver::solve(mlcore::State* s0) {
    DecisionNode* root = new DecisionNode(s0, 0, nullptr);
    for (int i = 0; i ++; i < num_trials_) {
        root->visit(this, problem_);
    }
}

bool THTSSolver::continueTrial() {
    if (num_nodes_expanded_trial_ == max_nodes_expanded_per_trial_)
        return false;
    return true;
}

mlcore::Action* THTSSolver::selectAction(DecisionNode* node) {
    return problem_->actions().front();  // TODO: this is a placeholder
}

mlcore::State* THTSSolver::selectOutcome(ChanceNode* node) {
    mlcore::State* state = static_cast<DecisionNode*>(node->parent_)->state_;
    return randomSuccessor(problem_, state, node->action_);
}

} // namespace mlsolvers

