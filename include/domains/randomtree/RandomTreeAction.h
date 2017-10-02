#ifndef MDPLIB_RANDOMTREEACTION_H
#define MDPLIB_RANDOMTREEACTION_H

#include <vector>

#include "RandomTreeState.h"

#include "../../Action.h"

class RandomTreeAction : public mlcore::Action {
private:
    // An unique identifier for the action.
    int index_;

    // The cost of the action.
    double cost_;

    virtual std::ostream& print(std::ostream& os) const {
        os << "A" << index_;
        return os;
    }

public:
    // Creates an action for the random tree problem with the given id and
    // cost.
    RandomTreeAction(int index, double cost) : index_(index), cost_(cost) {}

    virtual ~RandomTreeAction() {}

    int index() const { return index_; }

    double cost() const { return cost_; }

    // Override method from Action.
    virtual int hashValue() const;

    // Override method from Action.
    virtual mlcore::Action& operator=(const mlcore::Action& rhs);
};

#endif // MDPLIB_RANDOMTREEACTION_H
