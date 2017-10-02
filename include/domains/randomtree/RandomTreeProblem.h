#ifndef MDPLIB_RANDOMTREEPROBLEM_H
#define MDPLIB_RANDOMTREEPROBLEM_H

#include <random>

#include "RandomTreeState.h"
#include "../../Action.h"
#include "../../Problem.h"
#include "../../State.h"

enum RandomTreeType { RANDOM = 0, SKEWED_VAR = 1};

// Implements an MDP structured as a balanced tree of some depth where the goal
// is at one of the leaves.
class RandomTreeProblem : public mlcore::Problem {
private:
    mlcore::State* absorbing_;

    // The depth of the tree.
    int depth_;

    // The maximum number of actions per state.
    int max_num_actions_;

    // The maximum number of successors per action.
    int max_num_successors_;

    // Each new state and action is assigned a unique index.
    int current_index_;

    // The minimum cost of the actions.
    double min_cost_;

    // The maximum cost of the actions.
    double max_cost_;

    // The method by which the tree is created (default RANDOM).
    RandomTreeType type_;

    // A copy of the actions list, used for shuffling in place.
    std::vector<mlcore::Action*> actions_copy_;

    // Used to generate random costs.
    std::uniform_int_distribution<int> unif_costs_;

    // Used to generate random number of actions.
    std::uniform_int_distribution<int> unif_actions_;

    // Used to generate random number of successors.
    std::uniform_int_distribution<int> unif_successors_;

    // Generates a random transition distribution for the given number of
    // elements, and stores the value in the given vector.
    void generateRandomTransition(
        int num_elements, std::vector<double>& distribution);

    // Creates a state with a random number of actions. The method receives
    // the depth of the state and whether it should be a goal or not. It also
    // receives the index of this state in the successor array where it
    // appears.
    RandomTreeState* createRandomTreeState(
        int depth,
        bool is_goal,
        int cost_term = 0,
        int upper_bound_successor_cost_term = 0);

    // Creates max_num_actions_ each with a cost drawn from an
    // uniform distribution in [min_cost_, max_cost_].
    void createActions();

    // Creates all successors of the state, for each of its stored actions.
    void createAllSuccessorsState(RandomTreeState* rts);

    // Creates the successors for the given state and action.
    void createSuccessorsStateAction(RandomTreeState* rts,
                                     mlcore::Action* action);

    // Creates a random tree of states and actions rooted at the given state.
    void createRandomTree(RandomTreeState* root);

public:
    RandomTreeProblem(int depth,
                      RandomTreeType type = RANDOM,
                      int max_num_actions = 2,
                      int max_num_successors = 2,
                      int min_cost = 1,
                      int max_cost = 1);

    virtual ~RandomTreeProblem() { }

    // Override method from Problem
    virtual bool goal(mlcore::State* s) const;

    // Override method from Problem
    virtual
    mlcore::SuccessorsList transition(mlcore::State* s, mlcore::Action *a);

    // Override method from Problem
    virtual double cost(mlcore::State* s, mlcore::Action *a) const;

    // Override method from Problem
    virtual bool applicable(mlcore::State* s, mlcore::Action *a) const;
};

#endif // MDPLIB_RANDOMTREEPROBLEM_H
