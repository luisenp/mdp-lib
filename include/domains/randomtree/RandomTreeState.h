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
    int index_;

    // True only for goal states.
    bool is_goal_;

    // The depth at which this state appears in the random tree.
    int depth_;

    // The number of actions that are applicable in this state.
    int num_actions_;

    // The successors of this state for each of the applicable actions.
    ActionSuccessorsListMap action_successors_;

    virtual std::ostream& print(std::ostream& os) const {
        os << "S" << index_;
        return os;
    }

public:
    // Creates a state for the random tree with the given index, and depth.
    // The constructor also specifies if this state is a goal or not.
    RandomTreeState(int index, int depth, bool is_goal);

    virtual ~RandomTreeState() {}

    bool index() const { return index_; }

    bool isGoal() const { return is_goal_; }

    int depth() const { return depth_; }

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
