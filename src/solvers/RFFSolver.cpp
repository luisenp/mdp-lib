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
    mlcore::StateSet expandedStates;
    terminalStates_.insert(s0);

    mlcore::StateSet newTerminalStates;
    for (mlcore::State* s : terminalStates_) {
        // Solving using FF
                                                                                dprint1("Calling FF");
        vector<string> fullPlan;
        callFF(s, fullPlan);
                                                                                dprint2("fullplan", fullPlan.size());

        // Extract policy
        mlcore::State* sPrime = s;
        for (string actionName : fullPlan) {
                                                                                std::cerr << sPrime << "===" << actionName << "===";
            mlcore::Action* action = problem_->getActionFromName(actionName);
            if (action == nullptr) {
                sPrime->markDeadEnd();
                continue;
            }
            sPrime->setBestAction(action);
            expandedStates.insert(sPrime);
                                                                                std::cerr << action << std::endl;
            // Add new set of terminal states
            for (auto const succ : problem_->transition(sPrime, action)) {
                if (succ.su_state->bestAction() == nullptr ||
                    !problem_->goal(succ.su_state)) {
                    newTerminalStates.insert(succ.su_state);
                }
            }
            sPrime = mostLikelyOutcome(problem_, sPrime, action, true);
                                                                                std::cerr << "added all successors" << std::endl;
        }
    }
                                                                                std::cerr << "done with the terminal states" << std::endl;
                                                                                std::cerr << "newterm " << newTerminalStates.size() << std::endl;
                                                                                std::cerr << "expand " << expandedStates.size() << std::endl;
    terminalStates_.insert(newTerminalStates.begin(), newTerminalStates.end());
    for (mlcore::State* sRemove : expandedStates)
        terminalStates_.erase(sRemove);
                                                                                dprint2("terminalStates", terminalStates_.size());
    double totalProb = failProb(s0, 50);
                                                                                dprint2("totalProb", totalProb);
    return s0->bestAction();
}

void RFFSolver::callFF(mlcore::State* s, vector<string>& fullPlan) const
{
    string atoms = extractStateAtoms(static_cast<mlppddl::PPDDLState*> (s));
    replaceInitStateInProblemFile(templateProblemFilename_,
                                  atoms + removedInitAtoms_,
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
            currentState = randomSuccessor(problem_,
                                           currentState,
                                           currentState->bestAction());
        }
        if (terminalStates_.count(currentState))
            probabilitiesTerminals_[s] += delta;
        totalProbabilityTerminals += delta;
    }
    return totalProbabilityTerminals;
}

}
