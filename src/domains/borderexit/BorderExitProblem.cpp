#include "../../../include/domains/borderexit/BorderExitProblem.h"

const int BorderExitProblem::kActionExit = 0;
const int BorderExitProblem::kActionContinue = 1;
const int BorderExitProblem::kGoalId = 4;
const double BorderExitProblem::kExitProb = 0.6;


BorderExitProblem::BorderExitProblem() {
    s0 = new BorderExitState(0);
    this->addState(s0);
    absorbing_ = new BorderExitState(-1);
    this->addState(absorbing_);
    actions_.push_back(new BorderExitAction(kActionContinue));
    actions_.push_back(new BorderExitAction(kActionExit));
}

bool BorderExitProblem::goal(mlcore::State* s) const {
    if (s == absorbing_)
        return true;
    BorderExitState* state = static_cast<BorderExitState*>(s);
    return state->id() == kGoalId;
}


std::list<mlcore::Successor> BorderExitProblem::transition(mlcore::State* s,
                                                           mlcore::Action* a) {
    if (goal(s) || s == absorbing_) {
        std::list<mlcore::Successor> successors;
        successors.push_back(
            mlcore::Successor(this->addState(absorbing_), 1.0));
        return successors;
    }
    BorderExitState* state = static_cast<BorderExitState*>(s);
    BorderExitAction* action = static_cast<BorderExitAction*>(a);
    std::list<mlcore::Successor> successors;
    if (action->id() == kActionExit) {
        successors.push_back(mlcore::Successor(
            this->addState(new BorderExitState(kGoalId)), 1.0));
    } else {
        if (state->id() != 3) {
            successors.push_back(mlcore::Successor(
                this->addState(new BorderExitState(kGoalId)), kExitProb));
            successors.push_back(mlcore::Successor(
                this->addState(new BorderExitState(state->id() + 1)),
                                                   1.0 - kExitProb));
        } else {
            successors.push_back(mlcore::Successor(this->addState(state), 1.0));
        }
    }
    return successors;
}

double BorderExitProblem::cost(mlcore::State* s, mlcore::Action* a) const {
    BorderExitState* state = static_cast<BorderExitState*>(s);
    BorderExitAction* action = static_cast<BorderExitAction*>(a);
    if (action->id() == kActionExit)
        return 10.0 + 10000.0 * (state->id() == 3);
    return 1.0;
}

bool BorderExitProblem::applicable(mlcore::State* s, mlcore::Action* a) const {
    return true;
}
