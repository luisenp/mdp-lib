#include <vector>

#include "../../include/ppddl/PPDDLState.h"

#include "../../include/solvers/FFUtil.h"
#include "../../include/solvers/RFFSolver.h"

using namespace std;

namespace mlsolvers

{

// TODO: For now, the GFF set will be only the goal. Modify this.
mlcore::Action* RFFSolver::solve(mlcore::State* s0)
{
                                                                                mdplib_debug = true;
    startingPlanningTime_ = time(nullptr);
    terminalStates_.insert(s0);
    mlcore::StateSet policyGraphSeen;
    vector<mlcore::State*> statesPolicyGraph;

    for (int i = 0; i < 10; i++) {
        mlcore::StateSet expandedStates;
        mlcore::StateSet newTerminalStates;
        for (mlcore::State* s : terminalStates_) {
            // Solving using FF
            vector<string> fullPlan;
//                                                                                dprint2("call-FF with", s);
//                                                                                for (auto const pupu : statesPolicyGraph)
//                                                                                    dprint2("graph ", pupu);
            callFF(s, statesPolicyGraph, fullPlan);
//                                                                                dprint1("done");

            // Extract policy
            mlcore::State* sPrime = s;
            for (string actionName : fullPlan) {
                if (problem_->goal(sPrime))
                    continue;
                expandedStates.insert(sPrime);
                mlcore::Action* action =
                    problem_->getActionFromName(actionName);
//                                                                                dprint3("------expanding ", sPrime, actionName);
                if (action == nullptr) {
                    sPrime->markDeadEnd();
                    continue;
                }
                sPrime->setBestAction(action);
                // Add new set of terminal states
                for (auto const succ : problem_->transition(sPrime, action)) {
                    if (succ.su_state->bestAction() == nullptr &&
                        !problem_->goal(succ.su_state)) {
                        newTerminalStates.insert(succ.su_state);
                    }
                }
                sPrime = mostLikelyOutcome(problem_, sPrime, action, true);
            }
        }
//                                                                                for (auto const pupu : newTerminalStates)
//                                                                                    dprint2("new terminal", pupu);
//                                                                                for (auto const pupu : expandedStates) {
//                                                                                    dprint2("expanded ", pupu);
//                                                                                    if (pupu->bestAction())
//                                                                                        dprint1(pupu->bestAction());
//                                                                                    else
//                                                                                        dprint1("null");
//                                                                                }
        terminalStates_.insert(newTerminalStates.begin(),
                               newTerminalStates.end());
        for (mlcore::State* sExpanded : expandedStates) {
            terminalStates_.erase(terminalStates_.find(sExpanded));
            if (policyGraphSeen.insert(sExpanded).second)
                statesPolicyGraph.push_back(sExpanded);
        }
//                                                                                dprint2("terminalStates", terminalStates_.size());
//                                                                                dprint2("policy graph ", statesPolicyGraph.size());
        double totalProb = failProb(s0, 50);
//                                                                                dprint2("totalProb", totalProb);
//                                                                                dprint1("*************************************");
//                                                                                dprint1("*************************************");
        if (totalProb < rho_)
            break;
    }
    return s0->bestAction();
}


void RFFSolver::callFF(mlcore::State* s,
                       vector<mlcore::State*> subgoals,
                       vector<string>& fullPlan) const
{
    string atoms = extractStateAtoms(static_cast<mlppddl::PPDDLState*> (s));
    replaceInitStateInProblemFile(templateProblemFilename_,
                                  atoms + removedInitAtoms_,
                                  currentProblemFilename_);

    addSubGoalsToProblemFile(currentProblemFilename_,
                             subgoals,
                             problem_,
                             currentProblemFilename_);

    pair<string, int> actionNameAndCost =
        getActionNameAndCostFromFF(ffExecFilename_,
                                   determinizedDomainFilename_,
                                   currentProblemFilename_,
                                   startingPlanningTime_,
                                   maxPlanningTime_,
                                   &fullPlan);
}


double RFFSolver::failProb(mlcore::State* s, int N)
{
    for (mlcore::State* s : terminalStates_)
        probabilitiesTerminals_[s] = 0.0;
    double totalProbabilityTerminals = 0.0;
    double delta = 1.0 / N;
    for (int i = 0; i < N; i++) {
        mlcore::State* currentState = s;
        while (!problem_->goal(currentState) &&
               terminalStates_.count(currentState) == 0) {
//                                                                                dprint2(currentState, currentState->bestAction());
            currentState = randomSuccessor(problem_,
                                           currentState,
                                           currentState->bestAction());
        }
        if (terminalStates_.count(currentState) > 0) {
            probabilitiesTerminals_[s] += delta;
            totalProbabilityTerminals += delta;
        }
//                                                                                dprint1("*********************************");
    }
    return totalProbabilityTerminals;
}

}
