#include "../../include/domains/AODeterminization.h"
#include "../../include/domains/DummyAction.h"


AllOutcomesDeterminization::
        AllOutcomesDeterminization(mlcore::Problem* problem) {
    originalProblem_ = problem;
    problem->generateAll();
    int s_idx = 0;
    for (mlcore::State* s : problem->states()) {
        states_.insert(s);
        if (s == problem->initialState())
            this->s0 = s;
        stateIndexMap_[s] = s_idx;
        transitionGraph_.push_back(std::unordered_map<int, int>());
        allStates_.push_back(s);
        s_idx++;
    }
    int action_idx = 0;
    for (mlcore::State* s : problem->states()) {
        int s_idx = stateIndexMap_[s];
        for (mlcore::Action* a : problem->actions()) {
            if (!problem->applicable(s, a))
                continue;
            for (auto& successor : problem->transition(s, a)) {
                int s_prime_idx = stateIndexMap_[successor.su_state];
                transitionGraph_[s_idx][action_idx] = s_prime_idx;
                actionCosts_.push_back(problem->cost(s, a));
                actions_.push_back(new DummyAction(action_idx));
                actionsVector_.push_back(actions_.back());
                action_idx++;
            }
        }
    }
}

std::list<mlcore::Action*>
AllOutcomesDeterminization::actions(mlcore::State* s) const {
    int s_idx = stateIndexMap_.at(s);
    std::list<mlcore::Action*> stateActions;
    for (auto& entry : transitionGraph_.at(s_idx)) {
        stateActions.push_back(actionsVector_[entry.first]);
    }
    return stateActions;
}

bool AllOutcomesDeterminization::goal(mlcore::State* s) const {
    return originalProblem_->goal(s);
}

std::list<mlcore::Successor>
AllOutcomesDeterminization::transition(mlcore::State* s, mlcore::Action* a) {
    int s_idx = stateIndexMap_[s];
    DummyAction* dummya = static_cast<DummyAction*>(a);
    int s_prime_idx = transitionGraph_[s_idx][dummya->id()];
    std::list<mlcore::Successor> successors;
    successors.push_back(mlcore::Successor(allStates_[s_prime_idx], 1.0));
    return successors;
}

double
AllOutcomesDeterminization::cost(mlcore::State* s, mlcore::Action* a) const {
    DummyAction* dummya = static_cast<DummyAction*>(a);
    return actionCosts_[dummya->id()];
}

bool AllOutcomesDeterminization::
        applicable(mlcore::State* s, mlcore::Action* a) const {
    int s_idx = stateIndexMap_.at(s);
    DummyAction* dummya = static_cast<DummyAction*>(a);
    return transitionGraph_.at(s_idx).count(dummya->id()) > 0;
}
