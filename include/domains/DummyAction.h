#ifndef DUMMYACTION_H
#define DUMMYACTION_H

#include "../Action.h"

class DummyAction : public mlcore::Action {

private:

    int id_;

    virtual std::ostream& print(std::ostream& os) const {
        os << "Dummy Action: (" << id_ << ")";
        return os;
    }

public:
    DummyAction(int id) : id_(id) {}

    virtual ~DummyAction() {}

    int id() { return id_; }

    /**
     * Overriding method from Action.
     */
    virtual mlcore::Action& operator=(const mlcore::Action& rhs) {
        if (this == &rhs)
            return *this;

        const DummyAction* action = static_cast<const DummyAction*>(&rhs);
        id_ =  action->id_;
        return *this;
    }

    /**
     * Overriding method from Action.
     */
    virtual int hashValue() const {
        return id_;
    }
};

#endif // DUMMYACTION_H
