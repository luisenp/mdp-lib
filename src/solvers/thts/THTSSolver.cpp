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
//                                                                                dprint(debug_pad(2 * this->depth_), "backup-leaf", this, action_value_);
    } else {
        switch(solver->backup_function_) {
            case MONTE_CARLO: {
                action_value_ +=
                    (cumulative_value - action_value_) / backup_counter_;
                break;
            } case MAX_MONTE_CARLO: {
                double total_value_successors = 0.0;
                int total_backup_successors = 0;
                action_value_ = solver->problem_->cost(
                    static_cast<DecisionNode*>(this->parent_)->state_,
                    action_);
                for (DecisionNode* successor : explicated_successors_) {
                    total_value_successors +=
                        successor->backup_counter_ * successor->state_cost_;
                        total_backup_successors += successor->backup_counter_;
                }
                action_value_ +=
                    (total_value_successors / total_backup_successors);
                break;
            } case PARTIAL_BELLMAN: {
                double old_lower_bound = lower_bound_;
                double old_upper_bound = upper_bound_;

                double total_value_successors = 0.0;
                double total_prob_successors = 0;
                action_value_ = solver->problem_->cost(
                    static_cast<DecisionNode*>(this->parent_)->state_, action_);
                lower_bound_ = 0.0;
                upper_bound_ = 0.0;
                for (DecisionNode* successor : explicated_successors_) {
                    total_value_successors +=
                        successor->prob_ * successor->state_cost_;
                    lower_bound_ += successor->prob_ * successor->lower_bound_;
                    upper_bound_ += successor->prob_ * successor->upper_bound_;
                    total_prob_successors += successor->prob_;
                }
                action_value_ +=
                    (total_value_successors / total_prob_successors);

                double remaining_prob = (1 - total_prob_successors);
                lower_bound_ /= total_prob_successors;
                upper_bound_ /= total_prob_successors;
                lower_bound_ += solver->minCostNode(this) * remaining_prob;
                upper_bound_ += solver->maxCostNode(this) * remaining_prob;
                lower_bound_ += action_value_;
                upper_bound_ += action_value_;
//                                                                                dprint(debug_pad(2 * this->depth_), "backup", this, action_value_, total_value_successors, total_prob_successors);
                if (total_prob_solved_successors_ > 1.0 - mdplib::epsilon) {
                    this->solved_ = true;
                }
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
//                                                                                dprint(debug_pad(2 * this->depth_ + 1), "exists-dec", state, prob);
        // Successor state has already been explicated.
        int successor_node_index = state_successor_index_map_[state];
        assert(successor_node_index < explicated_successors_.size());
        return explicated_successors_[successor_node_index];
    } else {
//                                                                                dprint(debug_pad(2 * this->depth_ + 1), "create-dec", state, prob);
        // Successor state has never been explicated, add a new node.
        explicated_successors_.push_back(new DecisionNode(state,
                                                          this->depth_ + 1,
                                                          this->solver_,
                                                          this,
                                                          prob));
        explicated_successors_.back()->initialize(this->solver_);
        updateSuccessorIndexMap(state);
        return explicated_successors_.back();
    }
}

DecisionNode*
ChanceNode::getDecisionNodeForStateIfPresent(mlcore::State* state) {
    if (state_successor_index_map_.count(state)) {
        return explicated_successors_[state_successor_index_map_[state]];
    }
    return nullptr;
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

    lower_bound_ = solver->minCostNode(this);
    upper_bound_ = solver->maxCostNode(this);
    min_lower_bound_update_ = 1.0;
    max_lower_bound_update_ = 0.0;
    min_upper_bound_update_ = 1.0;
    max_upper_bound_update_ = 0.0;
}

double ChanceNode::visit(THTSSolver* solver) {
//                                                                                dprint(debug_pad(2 * this->depth_), "visit-chance", this, "from", this->parent());
    this->selection_counter_++;
    mlcore::State* s = static_cast<DecisionNode*>(this->parent_)->state_;
    double cumulative_value = solver->problem_->cost(s, action_);
    if (solver->continueTrial() && this->depth_ < solver->max_depth_) {
        double prob_successor = 0.0;
        mlcore::State* s = solver->selectOutcome(this, &prob_successor);
        DecisionNode* node = createOrGetDecisionNodeForState(s, prob_successor);
        cumulative_value += node->visit(solver);
    } else {
        // Assigns a value according to the successors' heuristic cost,
        // in case the horizon was not large enough for this problem
        for (mlcore::Successor successor
                : solver->problem_->transition(s, action_)) {
//                                                                                dprint(debug_pad(2 * this->depth_ + 1), "actioncost/su.prob/su.cost", solver->problem_->cost(s, action_), successor.su_prob, successor.su_state->cost());
            cumulative_value += successor.su_prob * successor.su_state->cost();
        }
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
    state_cost_ = 0.0;
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
    state_cost_ = std::numeric_limits<double>::max();
    double old_lower_bound = lower_bound_;
    double old_upper_bound = upper_bound_;
    this->solved_ = true;
    for (ChanceNode* successor : successors_) {
        this->solved_ &= successor->solved_;
//                                                                                if (parent_ == nullptr) dprint("BACKUP-ROOT, successor->solved", successor, successor->solved_);
        state_cost_  = std::min(state_cost_, successor->action_value_);
        lower_bound_ = std::min(lower_bound_, successor->lower_bound_);
        upper_bound_ = std::min(upper_bound_, successor->upper_bound_);
    }
    if (this->solved_) {
        updateParentWithSolvedOutcome();
    }

    // Update bounds statistics
    double max_change = old_upper_bound - old_lower_bound;
    double change_lower_bound = (lower_bound_ - old_lower_bound) / max_change;
    double change_upper_bound = (old_upper_bound - upper_bound_) / max_change;
    min_lower_bound_update_ =
        std::min(min_lower_bound_update_, change_lower_bound);
    max_lower_bound_update_ =
        std::max(max_lower_bound_update_, change_lower_bound);
    min_upper_bound_update_ =
        std::min(min_upper_bound_update_, change_upper_bound);
    max_upper_bound_update_ =
        std::min(max_upper_bound_update_, change_upper_bound);

//                                                                                dprint(debug_pad(2 * this->depth_), "backup", this, state_cost_, this->solved_);
}

double DecisionNode::visit(THTSSolver* solver) {
//                                                                                if (this->parent_) dprint(debug_pad(2 * this->depth_), "visit-dec", this, "from", this->parent_);
//                                                                                else dprint(debug_pad(2 * this->depth_), "visit-dec", this, "ROOT");
    this->selection_counter_++;
    if (solver->problem_->goal(state_)) {
        // Just do the "backup" here to avoid modifying the backup() method
        this->backup_counter_++;
        state_cost_ = lower_bound_ = upper_bound_ = 0.0;
        if (solver->backup_function_ == PARTIAL_BELLMAN) {
            assert(!this->solved_);
            updateParentWithSolvedOutcome();
            this->solved_ = true;
        }
//                                                                                dprint(debug_pad(2 * this->depth_), "goal");
        return 0.0 ;
    }
    if (successors_.empty()) {   // not expanded yet
        for (mlcore::Action* action : solver->problem_->actions()) {
            if (!solver->problem_->applicable(this->state_, action))
                continue;
            successors_.push_back(
                new ChanceNode(action, this->depth_, this->solver_, this));
            updateSuccessorIndexMap(action);
        }
        this->initialize(solver);
    }
    mlcore::Action* a = solver->selectAction(this);
    ChanceNode* chance_node = getChanceNodeForAction(a);
    double cumulative_value = chance_node->visit(solver);
//                                                                                dprint(debug_pad(2 * this->depth_), "selected-action/c. value", a, cumulative_value);
    this->backup(solver, cumulative_value);
    return cumulative_value;
}

void DecisionNode::initialize(THTSSolver* solver) {
    state_cost_ = std::numeric_limits<double>::max();
    for (ChanceNode* chance_node : successors_) {
        chance_node->initialize(solver);
        state_cost_ = std::min(state_cost_, chance_node->action_value_);
    }
    backup_counter_ = solver->num_virtual_rollouts_ * successors_.size();
    selection_counter_ = backup_counter_;
    solved_ = false;

    lower_bound_ = solver->minCostNode(this);
    upper_bound_ = solver->maxCostNode(this);
    min_lower_bound_update_ = 1.0;
    max_lower_bound_update_ = 0.0;
    min_upper_bound_update_ = 1.0;
    max_upper_bound_update_ = 0.0;
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
//                                                                                dprint("solve", num_trials_);
    root_ = make_unique<DecisionNode>(s0, 0, this, nullptr);
    for (int i = 0; i < num_trials_; i++) {
        if (root_.get()->solved_)
            break;
        root_.get()->visit(this);
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

void THTSSolver::greedyActionSelectionRule(
        DecisionNode* node, std::vector<ChanceNode*>& best_action_nodes) {
    double best_value = std::numeric_limits<double>::max();
    for (ChanceNode* action_node : node->successors()) {
        if (action_node->solved_) {  // Select only unsolved nodes
            continue;
        }
        if (action_node->action_value_ < best_value) {
            best_value = action_node->action_value_;
            best_action_nodes.clear();
        }
        if (fabs(action_node->action_value_ - best_value) < mdplib::epsilon) {
            best_action_nodes.push_back(action_node);
        }
    }
}

void
THTSSolver::ucb1ActionSelectRule(DecisionNode* node,
                                 double q_min,
                                 double q_max,
                                 std::vector<ChanceNode*>& best_action_nodes) {
    double best_value = std::numeric_limits<double>::max();
    double q_diff = q_max - q_min;
//                                                                                dprint(debug_pad(2 * node->depth_ + 1), "q-values (max, min, diff)", q_max, q_min, q_diff);
    for (ChanceNode* action_node : node->successors()) {
//                                                                                dprint(debug_pad(2 * node->depth_ + 1), "counters (node/action):", node->selection_counter_, action_node->selection_counter_);
        if (action_node->selection_counter_ == 0) {
            best_action_nodes.push_back(action_node);
            return;
        }
        double q_normalized = q_diff == 0 ?
            1 : (action_node->action_value_ - q_min) / (q_diff);
        double parent_visit_term = log(node->selection_counter_);
        double value_ucb1 =
            sqrt(parent_visit_term / action_node->selection_counter_);
        double q_ucb1 = q_normalized - value_ucb1;
//                                                                                dprint(debug_pad(2 * node->depth_ + 1), "action, q-normalized, ucb1_term, q_ucb1:", action_node->action_, q_normalized, value_ucb1, q_ucb1);

        if (action_node->solved_) {  // Select only unsolved nodes
//                                                                                dprint(debug_pad(2 * node->depth_ + 1), "tried-to-select-solved:", action_node);
//                                                                                dprint(debug_pad(2 * node->depth_ + 1), "action, q-normalized, ucb1_term, q_ucb1:", action_node->action_, q_normalized, value_ucb1, q_ucb1);
            continue;
        }

        if (q_ucb1 < best_value) {
            best_value = q_ucb1;
            best_action_nodes.clear();
        }
        if (fabs(q_ucb1 - best_value) < mdplib::epsilon) {
            best_action_nodes.push_back(action_node);
        }
    }
}

mlcore::Action* THTSSolver::selectAction(DecisionNode* node) {
    double q_min = std::numeric_limits<double>::max();
    double q_max = -q_min;
    for (ChanceNode* action_node : node->successors()) {
//                                                                                dprint(debug_pad(2 * node->depth_ + 1), action_node, "action-select-minmax:", action_node->action_value_);
        q_max = std::max(action_node->action_value_ , q_max);
        q_min = std::min(action_node->action_value_, q_min);
    }
    std::vector<ChanceNode*> best_action_nodes;
    switch (action_selection_) {
        case UCB1:
            ucb1ActionSelectRule(node, q_min, q_max, best_action_nodes);
            break;
        case GREEDY:
            greedyActionSelectionRule(node, best_action_nodes);
    }
    return best_action_nodes[rand() % best_action_nodes.size()]->action_;
}

mlcore::State*
THTSSolver::randomUnsolvedOutcomeSelect(ChanceNode* chance_node, double* prob) {
    mlcore::State* state =
        static_cast<DecisionNode*>(chance_node->parent_)->state_;
    double pick = mlsolvers::dis(mlsolvers::gen);
    double acc = 0.0;
    for (mlcore::Successor sccr :
            problem_->transition(state, chance_node->action_)) {
        DecisionNode* outcome_node =
            chance_node->getDecisionNodeForStateIfPresent(sccr.su_state);
        if (outcome_node != nullptr && outcome_node->solved_) {
            continue;
        }
        acc += sccr.su_prob
            / (1.0 - chance_node->total_prob_solved_successors_);
        if (acc >= pick) {
            if (prob != nullptr)
                *prob = sccr.su_prob;
            return sccr.su_state;
        }
    }
    assert(false);
    return nullptr;
}

mlcore::State* THTSSolver::VIUnifBoundsOutcomeSelect(
        ChanceNode* chance_node, double* prob) {
    double pick = mlsolvers::dis(mlsolvers::gen);
    if (pick < 0.25) {
        return randomUnsolvedOutcomeSelect(chance_node, prob);
    }
    // First find if this is the current best action or not.
    DecisionNode* parent = static_cast<DecisionNode*>(chance_node->parent_);
    double q_alpha = std::numeric_limits<double>::max();
    double best_qvalue_other_action = std::numeric_limits<double>::max();
    for (ChanceNode* other_chance_node : parent->successors_) {
        if (other_chance_node != chance_node) {
            if (other_chance_node->action_value_ < best_qvalue_other_action) {
                best_qvalue_other_action = other_chance_node->action_value_;
                q_alpha = (other_chance_node->upper_bound_
                            + other_chance_node->lower_bound_) / 2;
            }
        }
    }
    if (best_qvalue_other_action == std::numeric_limits<double>::max()) {
        // No other action, just sample as usual.
        return randomUnsolvedOutcomeSelect(chance_node, prob);
    }

    mlcore::State* state =
        static_cast<DecisionNode*>(chance_node->parent_)->state_;
    // Pre-compute the total cost estimate of this action (from the bounds)
    // so it's easier to compute the contribution of each outcome later.
    double qvalue_estimate = 0.0;
    double total_prob = 0.0;
    for (mlcore::Successor sccr :
            problem_->transition(state, chance_node->action_)) {
        DecisionNode* outcome_node =
            chance_node->getDecisionNodeForStateIfPresent(sccr.su_state);
                                                                                dprint("got-dec", (void *) outcome_node);
        if (outcome_node == nullptr || outcome_node->backup_counter_ < 2) {
                                                                                if (outcome_node != nullptr) dprint("  ", outcome_node->backup_counter_);
            // Not enough info to do VI, sample using normal transition.
            return randomUnsolvedOutcomeSelect(chance_node, prob);
        }
        qvalue_estimate += outcome_node->prob_
            * (outcome_node->upper_bound_ + outcome_node->lower_bound_) / 2;
        total_prob += outcome_node->prob_;
    }
    assert(fabs(total_prob - 1.0) < mdplib::epsilon);
    qvalue_estimate += problem_->cost(state, chance_node->action_);

    double best_outcome_score = -std::numeric_limits<double>::max();
    double total_score = 0.0;
    std::vector<double> scores;
    for (mlcore::Successor sccr :
            problem_->transition(state, chance_node->action_)) {
        DecisionNode* outcome_node =
            chance_node->getDecisionNodeForStateIfPresent(sccr.su_state);
        assert(outcome_node != nullptr);
        if (outcome_node->solved_)
            continue;
        double delta_bound =
            outcome_node->upper_bound_ - outcome_node->lower_bound_;
        double l1 = outcome_node->lower_bound_
            + delta_bound * outcome_node->min_lower_bound_update_;
        double h1 = l1;
        double h2 = outcome_node->upper_bound_
            - delta_bound * outcome_node->min_upper_bound_update_;
        double l2 = h2;
        double tau = qvalue_estimate
            - (outcome_node->upper_bound_ + outcome_node->lower_bound_) / 2;
        double z = 2 * (q_alpha - tau) / outcome_node->prob_;

        double xl = std::min(l2, std::max(l1, z - h2));
        double xh = std::max(l1, std::min(l2, z - h1));
        double yl = std::min(h2, std::max(h1, z - l2));
        double yh = std::max(h1, std::min(h2, z - l1));
        double xm = std::max(xl, std::min(z / 2, xh));
        double ym = std::min(yh, std::max(z / 2, yl));
//                                                                                dprint("qvalue", qvalue_estimate, "lb", outcome_node->lower_bound_, "ub", outcome_node->upper_bound_, "To", outcome_node->prob_, "tau", tau);
//                                                                                dprint("q_alpha", q_alpha);
//                                                                                dprint("delta", delta_bound, "l1", l1, "h2", h2, "z", z, "xl", xl, "xh", xh, "yl", yl, "yh", yh, "xm", xm, "ym", ym);
        double outcome_score = -1.0;
        if (qvalue_estimate > q_alpha) {
//                                                                                dprint("case I");
            // Current action is not the best, so an outcome only improves
            // policy if the cost of this action becomes lower than the cost
            // of the current best action.
            // Integrate from (l1, h1) to h = z - l, constrained to be l < h.
            double xl_minus_l1 = (xl - l1);
            double Q_alpha_minus_tau = q_alpha - tau;
            double term_1 = (h2 - xl) * xl_minus_l1
                * (Q_alpha_minus_tau
                    - outcome_node->prob_ * (l1 + h2 + 2 * xl) / 4 );
            double term_2 = xl_minus_l1 * xl_minus_l1
                * (Q_alpha_minus_tau
                    - outcome_node->prob_ * (l1 + xl) / 2) / 2;
            double term_3 = (xm - xl) * (xm - xl)
                * (Q_alpha_minus_tau
                    - outcome_node->prob_ * (xl + z) / 3);
//                                                                                dprint("term1", term_1, "term2", term_2, "term3", term_3);
            outcome_score = term_1 + term_2 + term_3;
            outcome_score = outcome_score / ((h2 - l1) * (h2 - l1));
        } else {
//                                                                                dprint("case II");
            // Current action is the best.
            // Integrate from h = z - l to (l2, h2), constrained to be l < h.
            double h2_minus_yh = (h2 - yh);
            double tau_minus_Q_alpha = tau - q_alpha;
            double term_1 = (yh - l1) * h2_minus_yh
                * (tau_minus_Q_alpha
                    + outcome_node->prob_ * (l1 + h2 + 2 * yh) / 4 );
            double term_2 = h2_minus_yh * h2_minus_yh
                * (tau_minus_Q_alpha
                    + outcome_node->prob_ * (yh + h2) / 2) / 2;
            double term_3 = (yh - ym) * (yh - ym)
                * (tau_minus_Q_alpha
                    + outcome_node->prob_ * (z + yh) / 3);
//                                                                                dprint("term1", term_1, "term2", term_2, "term3", term_3);
            outcome_score = term_1 + term_2 + term_3;
            outcome_score = outcome_score / ((h2 - l1) * (h2 - l1));

        }
//                                                                                dprint(outcome_score);
        total_score += outcome_score;
        scores.push_back(outcome_score);
        assert(outcome_score > -mdplib::epsilon);
        /*if (outcome_score > best_outcome_score) {
            best_outcome_score = outcome_score;
            best_states.clear();
        }
        if (fabs(outcome_score - best_outcome_score) < mdplib::epsilon) {
            best_states.push_back(sccr.su_state);
        }*/
    }


    double pick = mlsolvers::dis(mlsolvers::gen);
    double acc = 0.0;
    int index = 0;
    for (mlcore::Successor sccr :
            problem_->transition(state, chance_node->action_)) {
        DecisionNode* outcome_node =
            chance_node->getDecisionNodeForStateIfPresent(sccr.su_state);
        if (outcome_node->solved_)
            continue;
        if (total_score > 0)
            acc += scores[index++] / total_score;
        else acc += sccr.su_prob;
        if (acc >= pick) {
            if (prob != nullptr)
                *prob = sccr.su_prob;
            return sccr.su_state;
        }
    }
                                                                                dprint(total_score, "pick", pick);
                                                                                for (int i = 0; i < scores.size(); i++) {
                                                                                    dprint("score", scores[i]);
                                                                                }
    assert(false);
//    return best_states[rand() % best_states.size()];
}

mlcore::State* THTSSolver::VPIUnifOutcomeSelect(
        ChanceNode* chance_node, double* prob) {
    /*double pick = mlsolvers::dis(mlsolvers::gen);
    if (pick < 0.25) {
        return randomUnsolvedOutcomeSelect(chance_node, prob);
    }*/
    // First find if this is the current best action or not.
    DecisionNode* parent = static_cast<DecisionNode*>(chance_node->parent_);
    double best_qvalue_other_action = std::numeric_limits<double>::max();
    for (ChanceNode* other_chance_node : parent->successors_) {
        if (other_chance_node != chance_node) {
            double qvalue_estimate_other_action =
                (other_chance_node->upper_bound_
                    + other_chance_node->lower_bound_) / 2;
            best_qvalue_other_action = std::min(qvalue_estimate_other_action,
                                                best_qvalue_other_action);
        }
    }
    if (best_qvalue_other_action == std::numeric_limits<double>::max()) {
        // No other action, just sample as usual.
        return randomUnsolvedOutcomeSelect(chance_node, prob);
    }

    mlcore::State* state =
        static_cast<DecisionNode*>(chance_node->parent_)->state_;
    // Pre-compute the total cost estimate of this action (from the bounds)
    // so it's easier to compute the contribution of each outcome later.
    double qvalue_estimate = 0.0;
    double total_prob = 0.0;
    for (mlcore::Successor sccr :
            problem_->transition(state, chance_node->action_)) {
        DecisionNode* outcome_node =
            chance_node->getDecisionNodeForStateIfPresent(sccr.su_state);
                                                                                dprint("got-dec", (void *) outcome_node);
        if (outcome_node == nullptr || outcome_node->backup_counter_ < 2) {
                                                                                if (outcome_node != nullptr) dprint("  ", outcome_node->backup_counter_);
            // Not enough info to do VI, sample using normal transition.
            return randomUnsolvedOutcomeSelect(chance_node, prob);
        }
        qvalue_estimate += outcome_node->prob_
            * (outcome_node->upper_bound_ + outcome_node->lower_bound_) / 2;
        total_prob += outcome_node->prob_;
    }
    assert(fabs(total_prob - 1.0) < mdplib::epsilon);
    qvalue_estimate += problem_->cost(state, chance_node->action_);

    double best_outcome_score = -std::numeric_limits<double>::max();
    double total_score = 0.0;
    std::vector<double> scores;
    for (mlcore::Successor sccr :
            problem_->transition(state, chance_node->action_)) {
        DecisionNode* outcome_node =
            chance_node->getDecisionNodeForStateIfPresent(sccr.su_state);
        assert(outcome_node != nullptr);
        if (outcome_node->solved_)
            continue;
        double tau = qvalue_estimate
            - (outcome_node->upper_bound_ + outcome_node->lower_bound_) / 2;
        double z = (best_qvalue_other_action - tau) / outcome_node->prob_;

        double outcome_score = -1.0;
        if (qvalue_estimate > best_qvalue_other_action) {
            // Current action is not the best, so an outcome only improves
            // policy if the cost of this action becomes lower than the cost
            // of the current best action.
            // Integrate from lower_bound to z.
            outcome_score = (z - outcome_node->lower_bound_)
                * (best_qvalue_other_action - tau
                   - outcome_node->prob_
                    * (z + outcome_node->lower_bound_) / 2);
            outcome_score /=
                (outcome_node->upper_bound_ - outcome_node->lower_bound_);

        } else {
            // Current action is the best. Integrate from z to upper_bound.
            outcome_score = (outcome_node->upper_bound_ - z)
                * (tau - best_qvalue_other_action
                   + outcome_node->prob_
                    * (outcome_node->upper_bound_ + z) / 2);
            outcome_score /=
                (outcome_node->upper_bound_ - outcome_node->lower_bound_);
        }
        total_score += outcome_score;
        scores.push_back(outcome_score);
        assert(outcome_score > -mdplib::epsilon);
        /*if (outcome_score > best_outcome_score) {
            best_outcome_score = outcome_score;
            best_states.clear();
        }
        if (fabs(outcome_score - best_outcome_score) < mdplib::epsilon) {
            best_states.push_back(sccr.su_state);
        }*/
    }


    double pick = mlsolvers::dis(mlsolvers::gen);
    double acc = 0.0;
    int index = 0;
    for (mlcore::Successor sccr :
            problem_->transition(state, chance_node->action_)) {
        DecisionNode* outcome_node =
            chance_node->getDecisionNodeForStateIfPresent(sccr.su_state);
        if (outcome_node->solved_)
            continue;
        if (total_score > 0)
            acc += scores[index++] / total_score;
        else acc += sccr.su_prob;
        if (acc >= pick) {
            if (prob != nullptr)
                *prob = sccr.su_prob;
            return sccr.su_state;
        }
    }
                                                                                dprint(total_score, "pick", pick);
                                                                                for (int i = 0; i < scores.size(); i++) {
                                                                                    dprint("score", scores[i]);
                                                                                }
    assert(false);
//    return best_states[rand() % best_states.size()];
}

mlcore::State* THTSSolver::selectOutcome(ChanceNode* node, double* prob) {
    switch (outcome_selection_) {
        case TRAN_F: {
            return randomUnsolvedOutcomeSelect(node, prob);
        } case VI_UNIF_BOUNDS: {
            return VIUnifBoundsOutcomeSelect(node, prob);
        } case VPI_UNIF: {
            return VPIUnifOutcomeSelect(node, prob);
        }
    }
}

mlcore::Action* THTSSolver::recommend(DecisionNode* node) {
//                                                                                dprint("recommend");
    double best_value = std::numeric_limits<double>::max();
    std::vector<mlcore::Action*> best_actions;
    for (ChanceNode* chance_node : node->successors_) {
        double chance_node_value = std::numeric_limits<double>::max();
        switch(recommendation_function_) {
            case MOST_PLAYED: {
                chance_node_value = -chance_node->selection_counter_;
                break;
            } case BEST_VALUE: {
                chance_node_value = chance_node->action_value_;
                break;
            }
        }
        if (chance_node_value < best_value ) {
//                                                                                dprint("best-so-far", chance_node, chance_node_value);
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

