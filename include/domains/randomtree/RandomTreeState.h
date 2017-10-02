#ifndef MDPLIB_RANDOMTREESTATE_H
#define MDPLIB_RANDOMTREESTATE_H

#include <unordered_map>

#include "../../Action.h"
#include "../../State.h"

typedef std::unordered_map<mlcore::Action*,
                           mlcore::SuccessorsList,
                           mlcore::ActionHash,
                           mlcore::ActionEqual> ActionSuccessorsListMap;

class RandomTreeState : public mlcore::State {

private:
    // The unique identifier for this state.
    int id_;

    // True only for goal states.
    bool is_goal_;

    // The depth at which this state appears in the random tree.
    int depth_;

    // The number of actions that are applicable in this state.
    int num_actions_;

    // The successors of this state for each of the applicable actions.
    ActionSuccessorsListMap action_successors_;

    // A term that affects the MDP cost function
    // s.t. problem->cost(s,a) = cost_term_ + action_cost
    int cost_term_;

    // The upper bound for the successors cost terms.
    int upper_bound_successor_cost_term_;

    virtual std::ostream& print(std::ostream& os) const {
        os << "S" << id_;
        return os;
    }

public:
    // Creates a state for the random tree with the given id, and depth.
    // The constructor also specifies if this state is a goal or not, the
    // cost term and the upper bound for the successors cost terms.
    RandomTreeState(int id,
                    int depth,
                    bool is_goal,
                    int cost_term,
                    int upper_bound_successor_cost_term = -1);

    virtual ~RandomTreeState() {}

    bool id() const { return id_; }

    bool isGoal() const { return is_goal_; }

    int depth() const { return depth_; }

    int costTerm() const { return cost_term_; }

    int upperBoundSuccessorCostTerm() const
        { return upper_bound_successor_cost_term_; }

    ActionSuccessorsListMap& actionSuccessors()
        { return action_successors_; }

    // Adds an action for this state.
    void addAction(mlcore::Action* action);

    // Adds the given state as a successor to the given action, with the given
    // probability.
    void addSuccessorForAction(RandomTreeState* state,
                               double prob,
                               mlcore::Action* action);



    // Overrides operator from State.
    virtual State& operator=(const mlcore::State& rhs);

    // Overrides operator from State.
    virtual bool operator==(const mlcore::State& rhs) const;

    // Overrides method from State.
    virtual bool equals(mlcore::State* other) const;

    // Overrides method from State.
    virtual int hashValue() const;

};

#endif // MDPLIB_RANDOMTREESTATE_H
