#include <vector>

#include "../../include/ppddl/PPDDLState.h"

#include "../../include/solvers/FFUtil.h"
#include "../../include/solvers/RFFSolver.h"

using namespace std;

namespace mlsolvers

{

mlcore::Action* RFFSolver::solve(mlcore::State* s0)
{
    if (s0->bestAction() != nullptr)
        return s0->bestAction();
    startingPlanningTime_ = time(nullptr);
    terminalStates_.insert(s0);
    mlcore::StateSet statesPolicyGraph;

    for (int i = 0; i < 100; i++) {
        mlcore::StateSet expandedStates;
        mlcore::StateSet newTerminalStates;
        for (mlcore::State* s : terminalStates_) {
            // Solving using FF
            vector<string> fullPlan;
            vector<mlcore::State*> subgoals;
            if (!statesPolicyGraph.empty())
                pickRandomStates(statesPolicyGraph, 100, subgoals);
            callFF(s, subgoals, fullPlan);

            // Extract policy
            mlcore::State* sPrime = s;
            for (string actionName : fullPlan) {
                // Don't expand goal states or states that have been expanded
                // before
                if (problem_->goal(sPrime) ||
                        statesPolicyGraph.count(sPrime) > 0)
                    continue;
                expandedStates.insert(sPrime);
                mlcore::Action* action =
                    problem_->getActionFromName(actionName);
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
        terminalStates_.insert(newTerminalStates.begin(),
                               newTerminalStates.end());
        for (mlcore::State* sExpanded : expandedStates) {
            terminalStates_.erase(terminalStates_.find(sExpanded));
            statesPolicyGraph.insert(sExpanded);
        }
        double totalProb = failProb(s0, 50);
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
            if (currentState->deadEnd()) {
                // Treat dead-ends as goals, otherwise this method
                // might loop endlessly when there are unavoidable dead-ends
                break;
            }
            currentState = randomSuccessor(problem_,
                                           currentState,
                                           currentState->bestAction());
        }
        if (terminalStates_.count(currentState) > 0) {
            probabilitiesTerminals_[s] += delta;
            totalProbabilityTerminals += delta;
        }
    }
    return totalProbabilityTerminals;
}


void RFFSolver::pickRandomStates(mlcore::StateSet& states,
                                 int n,
                                 vector<mlcore::State*>& pickedStates)
{
    // If there are not n states to pick, just pick them all
    if (states.size() < n) {
        for (mlcore::State* s : states)
            pickedStates.push_back(s);
        return;
    }
    // Trick taken from RFF's original code.
    // Here we make a list of random indices from 0 to S-1 as follows:
    // Every time a pick is made, the range of values is decreased by 1, to
    // simulate the previous indices already taken out of the pool. But then we
    // increase the pick value for every previous pick that was lower, to
    // simulate the "space" that was taken by removing the previous lower pick
    list<size_t> pickedIdx;
    for (size_t pickedCnt = 0; pickedCnt < n; pickedCnt++) {
        size_t pick =
            static_cast<size_t> (rand() % (states.size() - pickedCnt));
        for (int idx : pickedIdx) {
            if (pick < idx)
                break;
            else
                pick++;
        }
        pickedIdx.push_back(pick);
    }
    // Now we just have to add the states with the chosen indices
    size_t stateCnt = 0;
    for (mlcore::State* s : states) {
        if (stateCnt == pickedIdx.front()) {
            pickedStates.push_back(s);
            pickedIdx.pop_front();
        }
        stateCnt++;
    }
}

}
