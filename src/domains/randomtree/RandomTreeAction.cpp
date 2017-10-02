#include "../../../include/domains/randomtree/RandomTreeAction.h"

int RandomTreeAction::hashValue() const {
    return id_;
}

mlcore::Action& RandomTreeAction::operator=(const mlcore::Action& rhs) {
    if (this == &rhs)
        return *this;
    const RandomTreeAction* rta = static_cast<const RandomTreeAction*>(&rhs);
    this->id_ = rta->id_;
    return *this;
}
