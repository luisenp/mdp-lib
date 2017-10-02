#include "../../../include/domains/randomtree/RandomTreeAction.h"

int RandomTreeAction::hashValue() const {
    return index_;
}

mlcore::Action& RandomTreeAction::operator=(const mlcore::Action& rhs) {
    if (this == &rhs)
        return *this;
    const RandomTreeAction* rta = static_cast<const RandomTreeAction*>(&rhs);
    this->index_ = rta->index_;
    return *this;
}
