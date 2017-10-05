#include "../../../include/domains/randomtree/RandomTreeProblem.h"

#include <algorithm>

#include "../../../include/domains/randomtree/RandomTreeAction.h"
#include "../../../include/domains/randomtree/RandomTreeState.h"
#include "../../../include/solvers/Solver.h"

using namespace std;

RandomTreeProblem::RandomTreeProblem(int depth,
                                     RandomTreeType type,
                                     int max_num_actions,
                                     int max_num_successors,
                                     int min_cost,
                                     int max_cost)
    : depth_(depth),
      type_(type),
      max_num_actions_(max_num_actions),
      max_num_successors_(max_num_successors),
      min_cost_(min_cost),
      max_cost_(max_cost),
      unif_costs_(min_cost, max_cost),
      unif_actions_(2, max_num_actions),
      unif_successors_(2, max_num_successors) {
    current_index_ = -1;
    createActions();
    absorbing_ = createRandomTreeState(depth + 1, false);
    int upper_bound_successor_cost_term = (1 << (min(depth_, 16)));
    s0 = createRandomTreeState(0, false, 0, upper_bound_successor_cost_term);
    createRandomTree(static_cast<RandomTreeState*> (s0));
}

void RandomTreeProblem::createRandomTree(RandomTreeState* root) {
    if (root->depth() == depth_)
        return;
    createAllSuccessorsState(root);
    for (auto& entry : root->actionSuccessors()) {
        for (mlcore::Successor& sccr : entry.second) {
            createRandomTree(static_cast<RandomTreeState*>(sccr.su_state));
        }
    }
}

void RandomTreeProblem::createActions() {
    switch (type_) {
        case RANDOM: {
            for (int i = 0; i < max_num_actions_; i++) {
                double cost = unif_costs_(mlsolvers::gen);
                actions_.push_back(new RandomTreeAction(i, cost));
                actions_copy_.push_back(actions_.back());
            }
            break;
        } case SKEWED_VAR: {
            actions_.push_back(new RandomTreeAction(1, 2));
            actions_.push_back(new RandomTreeAction(0, 1));
            break;
        }
    }
}

RandomTreeState*
RandomTreeProblem::createRandomTreeState(int depth,
                                         bool is_goal,
                                         int cost_term,
                                         int upper_bound_successor_cost_term) {
    RandomTreeState* new_state = static_cast<RandomTreeState*>(
        this->addState(new RandomTreeState(current_index_++,
                                           depth,
                                           is_goal,
                                           cost_term,
                                           upper_bound_successor_cost_term)));
    if (is_goal) {
        new_state->addAction(actions_.front());
    } else {
        switch(type_) {
            case RANDOM: {
                random_shuffle(actions_copy_.begin(), actions_copy_.end());
                int num_actions = unif_actions_(mlsolvers::gen);
                for (int i = 0; i < num_actions; i++) {
                    new_state->addAction(actions_copy_[i]);
                }
                break;
            } case SKEWED_VAR: {
                for (mlcore::Action* action : actions_) {
                    new_state->addAction(action);
                }
                break;
            }
        }
    }
    return new_state;
}


void RandomTreeProblem::generateRandomTransition(
        int num_elements, vector<double>& distribution) {
    switch(type_) {
        case RANDOM: {
            int index_high_prob = rand() % num_elements;
            for (int i = 0; i < num_elements; i++) {
                double value = (i == index_high_prob) ?
                    0.9 : 0.1 / (num_elements - 1);
                distribution.push_back(value);
            }
            break;
        } case SKEWED_VAR: {
            distribution.push_back(0.75);
            distribution.push_back(0.25);
            break;
        }
    }
}

void RandomTreeProblem::createAllSuccessorsState(RandomTreeState* rts) {
    ActionSuccessorsListMap& action_successors_ = rts->actionSuccessors();
    for (auto& entry : action_successors_) {
        createSuccessorsStateAction(rts, entry.first);
    }
}

void RandomTreeProblem::createSuccessorsStateAction(RandomTreeState* rts,
                                                    mlcore::Action* action) {
    if (rts->isGoal()) {
        rts->addSuccessorForAction(
            static_cast<RandomTreeState*>(absorbing_), 1.0, action);
    } else {
        bool is_goal = (rts->depth() == (depth_ - 1));
        switch(type_) {
            case RANDOM: {
                int num_successors = unif_successors_(mlsolvers::gen);
                vector<double> probabilities;
                generateRandomTransition(num_successors, probabilities);
                for (int i = 0; i < num_successors; i++) {
                    RandomTreeState* su_state =
                        createRandomTreeState(rts->depth() + 1, is_goal, i);
                    rts->addSuccessorForAction(
                        su_state, probabilities[i], action);
                }
                break;
            } case SKEWED_VAR: {
                int num_successors = 2;
                vector<double> probabilities;
                RandomTreeAction* rta = static_cast<RandomTreeAction*>(action);
                generateRandomTransition(num_successors, probabilities);
                for (int i = 0; i < num_successors; i++) {
                    int new_upper = (i == 0) ?
                        max(rts->upperBoundSuccessorCostTerm() / 4, 1)
                        : rts->upperBoundSuccessorCostTerm();
                    int new_cost_term = (i == 0) ? 1
                        : (max(rts->upperBoundSuccessorCostTerm() / 2, 1)
                            * (1 << rta->id()));
                    RandomTreeState* su_state = createRandomTreeState(
                        rts->depth() + 1, is_goal, new_cost_term, new_upper);
                    rts->addSuccessorForAction(
                        su_state, probabilities[i], action);
                }
                break;
            }
        }
    }
}

bool RandomTreeProblem::goal(mlcore::State* s) const {
    return static_cast<RandomTreeState*>(s)->isGoal();
}

mlcore::SuccessorsList
RandomTreeProblem::transition(mlcore::State* s, mlcore::Action *a) {
    RandomTreeState* rts = static_cast<RandomTreeState*> (s);
    return rts->actionSuccessors()[a];
}

double RandomTreeProblem::cost(mlcore::State* s, mlcore::Action *a) const {
    if (goal(s))
        return 0.0;
    return static_cast<RandomTreeState*>(s)->costTerm()
        + static_cast<RandomTreeAction*>(a)->cost();
}

bool RandomTreeProblem::applicable(mlcore::State* s, mlcore::Action *a) const {
    RandomTreeState* rts = static_cast<RandomTreeState*> (s);
    return (rts->actionSuccessors().count(a) > 0);
}
