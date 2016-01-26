#include "../../include/ppddl/ppddl_problem.h"
#include "../../include/ppddl/ppddl_state.h"
#include "../../include/ppddl/ppddl_action.h"

namespace mlppddl
{

Problem::Problem(problem_t* pProblem) : pProblem_(pProblem)
{
    pProblem_->instantiate_actions();
    pProblem_->flatten();
    state_t::initialize(*pProblem_);

    // Getting initial state for the problem
    for (int i = 0; i < DISP_SIZE; i++)
        display_[i].first = new state_t;

    pProblem_->initial_states(display_);
    s0 = new State(this);
    ((State *) s0)->setPState(*display_[0].first);
    this->addState(s0);

    actionList_t pActions = pProblem_->actionsT();
    for (int i = 0; i < pActions.size(); i++)
        actions_.push_back(new Action(pActions[i], i));
}

bool Problem::goal(mlcore::State* s) const
{
    State* state = (State *) s;
    return pProblem_->goal().holds(*state->pState());
}

std::list<mlcore::Successor> Problem::transition(mlcore::State* s,
                                                 mlcore::Action* a)
{
    std::list<mlcore::Successor> successors;

    Action* action = (Action *) a;
    State* state = (State *) s;

    pProblem_->expand(*action->pAction(), *state->pState(), display_);
    for (int i = 0; display_[i].second != Rational(-1); i++) {
        State* nextState = new State(this);
        nextState->setPState(*display_[i].first);
        successors.push_back(
            mlcore::Successor(this->addState(nextState),
                              display_[i].second.double_value()));
    }
    return successors;
}

double Problem::cost(mlcore::State* s, mlcore::Action* a) const
{
    Action* action = (Action *) a;
    State* state = (State *) s;

    return action->pAction()->cost(*state->pState());
}

bool Problem::applicable(mlcore::State* s, mlcore::Action* a) const
{
    Action* action = (Action *) a;
    State* state = (State *) s;

    return action->pAction()->enabled(*state->pState());
}

}
