#ifndef BORDEREXITPROBLEM_H
#define BORDEREXITPROBLEM_H

#include "../../Action.h"
#include "../../Problem.h"
#include "../../State.h"


class BorderExitAction : public mlcore::Action {
private:
    int id_;

    virtual std::ostream& print(std::ostream& os) const {
        os << "A(" << id_ << ")";
        return os;
    }

public:
    BorderExitAction(int id) : id_(id) { }

    virtual ~BorderExitAction() {}

    int id() { return id_; }

    /**
     * Overriding method from Action.
     */
    virtual mlcore::Action& operator=(const mlcore::Action& rhs) {
        if (this == &rhs)
            return *this;
        BorderExitAction* action = (BorderExitAction*)  & rhs;
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

class BorderExitState : public mlcore::State {
private:
    int id_;

    virtual std::ostream& print(std::ostream& os) const {
        os << "S(" << id_<< ")";
        return os;
    }

public:
    BorderExitState(int id) : id_(id) {}

    ~BorderExitState() {}

    int id() const { return id_; }

    /**
     * Overrides method from State.
     */
    virtual mlcore::State& operator=(const mlcore::State& rhs) {
        if (this == &rhs)
            return *this;

        const BorderExitState* state =
            static_cast<const BorderExitState*> (&rhs);
        id_ = state->id_;
        return *this;
    }

    /**
     * Overrides method from State.
     */
    virtual bool operator==(const mlcore::State& rhs) const {
        const BorderExitState* state =
            static_cast<const BorderExitState*> (&rhs);
        return id_ == state->id_;
    }

    /**
     * Overrides method from State.
     */
    virtual bool equals(mlcore::State* other) const {
        BorderExitState* state = static_cast<BorderExitState*> (other);;
        return *this ==  *state;
    }

    /**
     * Overrides method from State.
     */
    virtual int hashValue() const {
        return id_;
    }
};


class BorderExitProblem : public mlcore::Problem {
private:
    mlcore::State* absorbing_;

    static const int kActionExit;

    static const int kActionContinue;

    static const int kGoalId;

    static const double kExitProb;

public:
    BorderExitProblem();

    virtual ~BorderExitProblem() {}

    /**
     * Overrides method from Problem.
     */
    virtual bool goal(mlcore::State* s) const;

    /**
     * Overrides method from Problem.
     */
    virtual std::list<mlcore::Successor> transition(mlcore::State* s,
                                                    mlcore::Action* a);

    /**
     * Overrides method from Problem.
     */
    virtual double cost(mlcore::State* s, mlcore::Action* a) const;

    /**
     * Overrides method from Problem.
     */
    virtual bool applicable(mlcore::State* s, mlcore::Action* a) const;

};

#endif // BORDEREXITPROBLEM_H
