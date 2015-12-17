#include <vector>

#include "../../../include/mdplib.h"

#include "../../../include/solvers/metareasoning/MetareasoningAction.h"
#include "../../../include/solvers/metareasoning/MetareasoningProblem.h"
#include "../../../include/solvers/metareasoning/MetareasoningSimulator.h"
#include "../../../include/solvers/metareasoning/MetareasoningState.h"

MetareasoningProblem::MetareasoningProblem(mlsolvers::MetareasoningSimulator* simulator)
{
    simulator_ = simulator;
    mlcore::State* initialState = new MetareasoningState(simulator_->problem()->initialState(), 0);
    s0 = this->addState(initialState);
}


bool MetareasoningProblem::goal(mlcore::State* s) const
{
    MetareasoningState* metaState = (MetareasoningState *) s;
    return simulator_->problem()->goal(metaState->baseState());
}


std::list<mlcore::Successor> MetareasoningProblem::transition(mlcore::State* s, mlcore::Action* a)
{
    std::list<mlcore::Successor> successors;
    MetareasoningState* metaState = (MetareasoningState *) s;
    MetareasoningAction* metaAction = (MetareasoningAction *) a;

    if (metaAction->isNOP()) {
        int nextTime = std::min(metaState->iteration() + simulator_->numPlanningStepsPerNOP(),
                                (int) simulator_->stateValues().size() - 1);
        MetareasoningState* nextMetaState =
            new MetareasoningState(s, nextTime);
        successors.push_back(mlcore::Successor(this->addState(nextMetaState), 1.0));
    } else {
        // Adding successors of the greedy action in the base MDP.
        mlcore::Action* greedyActionChoice = getGreedyActionStateValue(metaState);
        for (mlcore::Successor su :
                simulator_->problem()->transition(metaState->baseState(), greedyActionChoice)) {
            int nextTime =
                std::min(metaState->iteration() + simulator_->numPlanningStepsPerAction(),
                         (int) simulator_->stateValues().size() - 1);
            MetareasoningState* nextMetaState = new MetareasoningState(su.su_state, nextTime);
            successors.push_back(mlcore::Successor(this->addState(nextMetaState), su.su_prob));
        }
    }

    return successors;
}


double MetareasoningProblem::cost(mlcore::State* s, mlcore::Action* a) const
{
    MetareasoningAction* metaAction = (MetareasoningAction *) a;
    if (metaAction->isNOP())
        return simulator_->costNOP();
    MetareasoningState* metaState = (MetareasoningState *) s;
    mlcore::Action* greedyActionChoice = getGreedyActionStateValue(metaState);
    return simulator_->problem()->cost(metaState->baseState(), greedyActionChoice);
}


bool MetareasoningProblem::applicable(mlcore::State* s, mlcore::Action* a) const
{
    MetareasoningState* metaState = (MetareasoningState *) s;
    MetareasoningAction* metaAction = (MetareasoningAction *) a;

    if (metaAction->isNOP())
        return true;
    mlcore::Action* greedyActionChoice = getGreedyActionStateValue(metaState);
    return simulator_->problem()->applicable(metaState->baseState(), greedyActionChoice);
}


mlcore::Action* MetareasoningProblem::getGreedyActionStateValue(MetareasoningState* s) const
{
    mlcore::Problem* problem = simulator_->problem();
    MetareasoningState* metaState = (MetareasoningState *) s;
    mlcore::State* baseState = metaState->baseState();

    // Finding greedy action according to the current VI values.
    double bestQValue = mdplib::dead_end_cost + 1;
    mlcore::Action* greedyActionChoice = nullptr;
    for (mlcore::Action* a : problem->actions()) {
        double qValue = 0.0;
        for (mlcore::Successor su :
                problem->transition(baseState, a)) {
            qValue += su.su_prob * simulator_->stateValues()[metaState->iteration()][su.su_state];
        }
        qValue = (qValue * problem->gamma()) + problem->cost(baseState, a);
        if (qValue < bestQValue) {
            bestQValue = qValue;
            greedyActionChoice = a;
        }
    }

    return greedyActionChoice;
}
