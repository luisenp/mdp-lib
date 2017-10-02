#include "../../../include/domains/randomtree/RandomTreeProblem.h"

#include <algorithm>

#include "../../../include/domains/randomtree/RandomTreeAction.h"
#include "../../../include/domains/randomtree/RandomTreeState.h"
#include "../../../include/solvers/Solver.h"

using namespace std;

RandomTreeProblem::RandomTreeProblem(int depth,
                                     int max_num_actions,
                                     int max_num_successors,
                                     int min_cost,
                                     int max_cost)
    : depth_(depth),
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
    s0 = createRandomTreeState(0, false);
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
    for (int i = 0; i < max_num_actions_; i++) {
        double cost = unif_costs_(mlsolvers::gen);
        actions_.push_back(new RandomTreeAction(i, cost));
        actions_copy_.push_back(actions_.back());
    }
}

RandomTreeState*
RandomTreeProblem::createRandomTreeState(int depth, bool is_goal) {
    RandomTreeState* new_state = static_cast<RandomTreeState*>(
        this->addState(new RandomTreeState(current_index_++, depth, is_goal)));
    if (is_goal) {
        new_state->addAction(actions_.front());
    } else {
        random_shuffle(actions_copy_.begin(), actions_copy_.end());
        int num_actions = unif_actions_(mlsolvers::gen);
        for (int i = 0; i < num_actions; i++) {
            new_state->addAction(actions_copy_[i]);
        }
    }
    return new_state;
}


void RandomTreeProblem::generateRandomTransition(
        int num_elements, vector<double>& distribution) {
    int index_high_prob = rand() % num_elements;
    for (int i = 0; i < num_elements; i++) {
        double value = (i == index_high_prob) ? 0.9 : 0.1 / (num_elements - 1);
        distribution.push_back(value);
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
        int num_successors = unif_successors_(mlsolvers::gen);
        vector<double> probabilities;
        generateRandomTransition(num_successors, probabilities);
        for (int i = 0; i < num_successors; i++) {
            bool is_goal = (rts->depth() == (depth_ - 1));
            RandomTreeState* su_state =
                createRandomTreeState(rts->depth() + 1, is_goal);
            rts->addSuccessorForAction(su_state, probabilities[i], action);
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
    return static_cast<RandomTreeAction*>(a)->cost();
}

bool RandomTreeProblem::applicable(mlcore::State* s, mlcore::Action *a) const {
    RandomTreeState* rts = static_cast<RandomTreeState*> (s);
    return (rts->actionSuccessors().count(a) > 0);
}
