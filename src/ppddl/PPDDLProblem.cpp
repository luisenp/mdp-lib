#include <sstream>

#include "../../include/ppddl/PPDDLAction.h"
#include "../../include/ppddl/PPDDLProblem.h"
#include "../../include/ppddl/PPDDLState.h"


namespace mlppddl
{

PPDDLProblem::PPDDLProblem(problem_t* pProblem) : pProblem_(pProblem)
{
    pProblem_->instantiate_actions();
    pProblem_->flatten();
    state_t::initialize(*pProblem_);

    // Getting initial state for the problem
    for (int i = 0; i < DISP_SIZE; i++)
        display_[i].first = new state_t;

    pProblem_->initial_states(display_);
    s0 = new PPDDLState(this);
    ((PPDDLState *) s0)->setPState(*display_[0].first);
    this->addState(s0);

    actionList_t pActions = pProblem_->actionsT();
    for (int i = 0; i < pActions.size(); i++)
        actions_.push_back(new PPDDLAction(pActions[i], i));
}


bool PPDDLProblem::goal(mlcore::State* s) const
{
    PPDDLState* state = (PPDDLState *) s;
    return pProblem_->goal().holds(*state->pState());
}


std::list<mlcore::Successor>
    PPDDLProblem::transition(mlcore::State* s, mlcore::Action* a)
{
    std::list<mlcore::Successor> successors;

    PPDDLAction* action = (PPDDLAction *) a;
    PPDDLState* state = (PPDDLState *) s;

    pProblem_->expand(*action->pAction(), *state->pState(), display_);
    for (int i = 0; display_[i].second != Rational(-1); i++) {
        PPDDLState* nextState = new PPDDLState(this);
        nextState->setPState(*display_[i].first);
        successors.push_back(
            mlcore::Successor(this->addState(nextState),
                              display_[i].second.double_value()));
    }
    return successors;
}


double PPDDLProblem::cost(mlcore::State* s, mlcore::Action* a) const
{
    PPDDLAction* action = (PPDDLAction *) a;
    PPDDLState* state = (PPDDLState *) s;

    return action->pAction()->cost(*state->pState());
}


bool PPDDLProblem::applicable(mlcore::State* s, mlcore::Action* a) const
{
    PPDDLAction* action = (PPDDLAction *) a;
    PPDDLState* state = (PPDDLState *) s;

    return action->pAction()->enabled(*state->pState());
}


mlcore::Action* PPDDLProblem::getActionFromName(std::string actionName)
{
    std::cerr << "a " << actionName << std::endl;
    size_t sepIdx = actionName.find("-xxdetxx-");
    if (sepIdx != std::string::npos) {
        std::string parameters = actionName.substr(sepIdx);
        parameters = parameters.substr(parameters.find(" "));
        actionName = actionName.substr(0, sepIdx) + parameters;
    }
    std::cerr << "b " << actionName << std::endl;
    std::ostringstream oss;
    for (mlcore::Action* a : this->actions()) {
        oss.str("");
        oss.clear();
        oss << a;
        if (oss.str().find(actionName) != std::string::npos)
            return a;
    }
    return nullptr;
}

}
