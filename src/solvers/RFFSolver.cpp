#include <vector>

#include "../../include/ppddl/PPDDLState.h"

#include "../../include/solvers/FFUtil.h"
#include "../../include/solvers/RFFSolver.h"

using namespace std;

namespace mlsolvers

{

mlcore::Action* RFFSolver::solve(mlcore::State* s0)
{
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
    for (string actionName : fullPlanFF) {
        mlcore::Action* action = problem_->getActionFromName(actionName);
        std::cerr << currentState << " " << actionName << std::endl;
        currentState =
            mostLikelyOutcome(problem_, currentState, action, true);
    }

}

}
