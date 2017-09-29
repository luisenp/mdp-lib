#include "../../../include/solvers/thts/THTSSolver.h"

#include <limits>

#include "../../../include/util/general.h"

using namespace std;

namespace mlsolvers
{

// ********************************************************************** //
//                               ChanceNode                               //
// ********************************************************************** //
ChanceNode::ChanceNode(mlcore::Action* action,
                       int depth,
                       THTSSolver* solver,
                       DecisionNode* parent) {
    assert(parent != nullptr);
    this->parent_ = parent;
    this->depth_ = depth;
    this->solver_ = solver;
    this->solved_ = false;
    action_ = action;
    action_value_ = 0.0;
    total_prob_solved_successors_ = 0.0;
    solver->register_node(this);
}

ChanceNode::~ChanceNode() {
    for (DecisionNode* successor : explicated_successors_)
        delete successor;
}

void ChanceNode::backup(THTSSolver* solver, double cumulative_value) {
    this->backup_counter_++;
    if (this->depth_ == solver->max_depth_) {
        // This is set to C(s,a) + sum_s' T(s'|s,a)H(s') by ChanceNode::visit()
        action_value_ = cumulative_value;
    } else {
        if (solver->backup_function_ == MONTE_CARLO) {
            // Monte-Carlo backup
            action_value_ +=
                (cumulative_value - action_value_) / backup_counter_;
        } else if (solver->backup_function_ == MAX_MONTE_CARLO) {
            // Max-Monte-Carlo backup
            double total_value_successors = 0.0;
            int total_backup_successors = 0;
            for (DecisionNode* successor : explicated_successors_) {
                total_value_successors +=
                    successor->backup_counter_ * successor->state_value_;
                    total_backup_successors += successor->backup_counter_;
            }
            action_value_ += (total_value_successors / total_backup_successors);
        } else if (solver->backup_function_ == PARTIAL_BELLMAN) {
            // Partial Bellman backup
            double total_value_successors = 0.0;
            double total_prob_successors = 0;
            for (DecisionNode* successor : explicated_successors_) {
                total_value_successors +=
                    successor->prob_ * successor->state_value_;
                total_prob_successors += successor->prob_;
            }
            action_value_ += (total_value_successors / total_prob_successors);
            if (total_prob_solved_successors_ > 1.0 - mdplib::epsilon) {
                this->solved_ = true;
            }
        }

    }
}

void ChanceNode::updateSuccessorIndexMap(mlcore::State* state) {
    state_successor_index_map_[state] = (explicated_successors_.size() - 1);
}

DecisionNode*
ChanceNode::createOrGetDecisionNodeForState(mlcore::State* state, double prob) {
    if (state_successor_index_map_.count(state)) {
        // Successor state has already been explicated.
        int successor_node_index = state_successor_index_map_[state];
        assert(successor_node_index < explicated_successors_.size());
        return explicated_successors_[successor_node_index];
    } else {
        // Successor state has never been explicated, add a new node.
        explicated_successors_.push_back(new DecisionNode(state,
                                                          this->depth_ + 1,
                                                          this->solver_,
                                                          this,
                                                          prob));
//                                                                                cerr << "create-new." << " " << explicated_successors_.back()->solved_ << endl;
        updateSuccessorIndexMap(state);
        return explicated_successors_.back();
    }
}

void ChanceNode::initialize(THTSSolver* solver) {
    if (solver->heuristic_ == nullptr) {
        action_value_ = 0.0;
    } else {
        action_value_ = solver->heuristic_->value(
            static_cast<DecisionNode*>(this->parent_)->state_, action_);
    }
    selection_counter_ = solver->num_virtual_rollouts_;
    backup_counter_ = solver->num_virtual_rollouts_;
    solved_ = false;
}

double ChanceNode::visit(THTSSolver* solver, mlcore::Problem* problem) {
                                                                                dprint5(debug_pad(2 * this->depth_), "visit-chance", this, "from", this->parent());
    this->selection_counter_++;
    mlcore::State* s = static_cast<DecisionNode*>(this->parent_)->state_;
    double cumulative_value = problem->cost(s, action_);
    if (solver->continueTrial() && this->depth_ < solver->max_depth_) {
        double prob_successor = 0.0;
        mlcore::State* s = solver->selectOutcome(this, &prob_successor);
        DecisionNode* node = createOrGetDecisionNodeForState(s, prob_successor);
//                                                                                cerr << "outcome: " << node << " solved " << node->solved_ << endl;
        cumulative_value += node->visit(solver, problem);
    } else {
        // Assigns a value according to the successors' heuristic cost,
        // in case the horizon was not large enough for this problem
        for (mlcore::Successor successor : problem->transition(s, action_))
            cumulative_value += successor.su_prob * successor.su_state->cost();
    }
    this->backup(solver, cumulative_value);
    return cumulative_value;
}

// ********************************************************************** //
//                               DecisionNode                             //
// ********************************************************************** //
DecisionNode::DecisionNode(mlcore::State* state,
                           int depth,
                           THTSSolver* solver,
                           ChanceNode* parent,
                           double prob) {
    if (parent == nullptr)
        assert(depth == 0);
    this->parent_ = parent;
    this->depth_ = depth;
    this->solved_ = false;
    state_ = state;
    state_value_ = 0.0;
    prob_ = prob;
    solver_ = solver;
    solver->register_node(this);
}

DecisionNode::~DecisionNode() {
    for (ChanceNode* successor : successors_)
        delete successor;
}

ChanceNode* DecisionNode::getChanceNodeForAction(mlcore::Action* action) {
    // For decision nodes, all successor chance nodes must be explicated.
    assert(action_chance_node_index_map_.count(action));
    int chance_node_index = action_chance_node_index_map_[action];
    assert(chance_node_index < successors_.size());
    return successors_[chance_node_index];
}

void DecisionNode::updateSuccessorIndexMap(mlcore::Action* action) {
    action_chance_node_index_map_[action] = (successors_.size() - 1);
}

void DecisionNode::backup(THTSSolver* solver, double cumulative_value) {
    this->backup_counter_++;
    state_value_ = std::numeric_limits<double>::max();
    this->solved_ = true;
    for (ChanceNode* successor : successors_) {
        this->solved_ &= successor->solved_;
        if (successor->action_value_ < state_value_) {
            state_value_ = successor->action_value_;
        }
    }
    if (this->solved_) {
        UpdateParentWithSolvedOutcome();
    }
}

double DecisionNode::visit(THTSSolver* solver, mlcore::Problem* problem) {
                                                                                if (this->parent_)
                                                                                    dprint5(debug_pad(2 * this->depth_), "visit-dec", this, "from", this->parent_);
    this->selection_counter_++;
    if (problem->goal(state_)) {
        // Just do the "backup" here to avoid modifying the backup() method
        this->backup_counter_++;
        state_value_ = 0.0;
        if (solver->backup_function_ == PARTIAL_BELLMAN) {
            assert(!this->solved_);
            UpdateParentWithSolvedOutcome();
            this->solved_ = true;
        }
        return 0.0 ;
    }
    if (successors_.empty()) {   // not expanded yet
        for (mlcore::Action* action : problem->actions()) {
            if (!problem->applicable(this->state_, action))
                continue;
            successors_.push_back(
                new ChanceNode(action, this->depth_, this->solver_, this));
            updateSuccessorIndexMap(action);
            this->initialize(solver);
        }
    }
    mlcore::Action* a = solver->selectAction(this);
    ChanceNode* chance_node = getChanceNodeForAction(a);
    double cumulative_value = chance_node->visit(solver, problem);
                                                                                dprint4(debug_pad(2 * this->depth_), "selected-action/c. value", a, cumulative_value);
    this->backup(solver, cumulative_value);
    return cumulative_value;
}

void DecisionNode::initialize(THTSSolver* solver) {
    state_value_ = std::numeric_limits<double>::max();
    for (ChanceNode* chance_node : successors_) {
        chance_node->initialize(solver);
        state_value_ = std::min(state_value_, chance_node->action_value_);
    }
    backup_counter_ = solver->num_virtual_rollouts_ * successors_.size();
    selection_counter_ = backup_counter_;
    solved_ = false;
}


// ********************************************************************** //
//                               THTSSolver                               //
// ********************************************************************** //
void THTSSolver::register_node(THTSNode* node) {
    node->index(current_node_index_);
    current_node_index_++;
}

// TODO: This needs to be adjusted so that old values can be reused during
// re-planning
mlcore::Action* THTSSolver::solve(mlcore::State* s0) {
                                                                                dprint2("solve", num_trials_);
    root_ = make_unique<DecisionNode>(s0, 0, this, nullptr);
    for (int i = 0; i < num_trials_; i++) {
        root_.get()->visit(this, problem_);
    }
    mlcore::Action* action = recommend(root_.get());
    delete_tree();
    return action;
}

bool THTSSolver::continueTrial() {
    if (num_nodes_expanded_trial_ == max_nodes_expanded_per_trial_)
        return false;
    return true;
}

void
THTSSolver::ucb1ActionSelectRule(DecisionNode* node,
                                 double q_min,
                                 double q_max,
                                 std::vector<ChanceNode*>& best_action_nodes) {
    double best_value = std::numeric_limits<double>::max();
    double q_diff = q_max - q_min;
                                                                                dprint5(debug_pad(2 * node->depth_ + 1), "q-values (max, min, diff)", q_max, q_min, q_diff);
    for (ChanceNode* action_node : node->successors()) {
                                                                                dprint3(debug_pad(2 * node->depth_ + 1), node->selection_counter_, action_node->selection_counter_);
        if (action_node->solved_) {  // Select only unsolved nodes
//                                                                                cerr << "tried-to-select-solved: " << action_node << endl;
            continue;
        }
        if (action_node->selection_counter_ == 0) {
            best_action_nodes.push_back(action_node);
            return;
        }
        double q_normalized = q_diff == 0 ?
            1 : (action_node->action_value_ - q_min) / (q_diff);
        double value_ucb1 = sqrt(2 * log(node->selection_counter_)
                                    / action_node->selection_counter_);
                                                                                dprint5(debug_pad(2 * node->depth_ + 1), "action, q-normalized, value_ucb1:", action_node->action_, q_normalized, value_ucb1);
                                                                                dprint4(debug_pad(2 * node->depth_ + 1), "counters (node/action):", node->selection_counter_, action_node->selection_counter_);
        double q_ucb1 = q_normalized - value_ucb1;
        if (q_ucb1 < best_value) {
            best_value = q_ucb1;
            best_action_nodes.clear();
        }
        if (q_ucb1 == best_value) {
            best_action_nodes.push_back(action_node);
        }
    }
}

mlcore::Action* THTSSolver::selectAction(DecisionNode* node) {
    double q_min = std::numeric_limits<double>::max();
    double q_max = -q_min;
    for (ChanceNode* action_node : node->successors()) {
        q_max = std::max(action_node->action_value_ , q_max);
        q_min = std::min(action_node->action_value_, q_min);
    }
    // UCB1 selection
    std::vector<ChanceNode*> best_action_nodes;
    ucb1ActionSelectRule(node, q_min, q_max, best_action_nodes);
    return best_action_nodes[rand() % best_action_nodes.size()]->action_;
}

mlcore::State*
THTSSolver::randomUnsolvedOutcomeSelect(ChanceNode* node, double* prob) {
    mlcore::State* state = static_cast<DecisionNode*>(node->parent_)->state_;
    double pick = mlsolvers::dis(mlsolvers::gen);
    double acc = 0.0;
//                                                                                cerr << "*************************************" << endl;
    for (mlcore::Successor sccr : problem_->transition(state, node->action_)) {
        DecisionNode* sccr_node = nullptr;
        if (node->state_successor_index_map_.count(sccr.su_state)) {
            sccr_node = node->explicated_successors_[
                node->state_successor_index_map_[sccr.su_state]];
        }
        if (sccr_node != nullptr && sccr_node->solved_) {
//                                                                                cerr << sccr_node << " " << sccr.su_prob << endl;
            continue;
        }
        acc += sccr.su_prob / (1.0 - node->total_prob_solved_successors_);
        if (acc >= pick) {
            if (prob != nullptr)
                *prob = sccr.su_prob;
            return sccr.su_state;
        }
    }
//                                                                                cerr << node << " pick " << pick << " acc " << acc
//                                                                                    << " totalP " << node->total_prob_solved_successors_
//                                                                                    << " solved " << node->solved_ << endl;
                                                                                assert(false);
    return nullptr;
}

mlcore::State* THTSSolver::selectOutcome(ChanceNode* node, double* prob) {
    return randomUnsolvedOutcomeSelect(node, prob);
}

mlcore::Action* THTSSolver::recommend(DecisionNode* node) {
                                                                                dprint1("recommend");
    double best_value = std::numeric_limits<double>::max();
    std::vector<mlcore::Action*> best_actions;
    for (ChanceNode* chance_node : node->successors_) {
        double chance_node_value = -chance_node->selection_counter_;
        if (chance_node_value < best_value ) {
            best_value = chance_node_value;
            best_actions.clear();
        }
        if (chance_node_value == best_value) {
            best_actions.push_back(chance_node->action_);
        }
    }
    return best_actions[rand() % best_actions.size()];
}

} // namespace mlsolvers

