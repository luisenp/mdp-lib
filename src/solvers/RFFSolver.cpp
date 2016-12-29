#include <vector>

#include "../../include/ppddl/PPDDLState.h"

#include "../../include/solvers/FFUtil.h"
#include "../../include/solvers/RFFSolver.h"

using namespace std;

namespace mlsolvers

{

mlcore::Action* RFFSolver::solve(mlcore::State* s0)
{
                                                                                mdplib_debug = true;
    startingPlanningTime_ = time(nullptr);
    string stateAtoms =
        extractStateAtoms(static_cast<mlppddl::PPDDLState*> (s0));

    replaceInitStateInProblemFile(templateProblemFilename_,
                                  stateAtoms + removedInitAtoms_,
                                  currentProblemFilename_);
    vector<string> fullPlanFF;
    pair<string, int> actionNameAndCost =
        getActionNameAndCostFromFF(ffExecFilename_,
                                   determinizedDomainFilename_,
                                   currentProblemFilename_,
                                   startingPlanningTime_,
                                   maxPlanningTime_,
                                   &fullPlanFF);

    mlcore::State* currentState = s0;
                                                                                dprint2("here5", fullPlanFF.size());
    for (string actionName : fullPlanFF) {
                                                                                dprint1("here6");
        std::cerr << currentState << "===" << actionName << "===";
        mlcore::Action* action = problem_->getActionFromName(actionName);
        currentState->setBestAction(action);
        std::cerr << action << std::endl;
        currentState =
            mostLikelyOutcome(problem_, currentState, action, true);
                                                                                dprint1("here7");
    }
                                                                                dprint3("here8", s0, actionNameAndCost.first);
    return problem_->getActionFromName(actionNameAndCost.first);

}

}
