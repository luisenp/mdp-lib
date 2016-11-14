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
                                                                                dprint1("here1");
    string stateAtoms =
        extractStateAtoms(static_cast<mlppddl::PPDDLState*> (s0));
                                                                                dprint1("here2");

    replaceInitStateInProblemFile(templateProblemFilename_,
                                  stateAtoms + removedInitAtoms_,
                                  currentProblemFilename_);
                                                                                dprint1("here3");
    vector<string> fullPlanFF;
    pair<string, int> actionNameAndCost =
        getActionNameAndCostFromFF(ffExecFilename_,
                                   determinizedDomainFilename_,
                                   currentProblemFilename_,
                                   startingPlanningTime_,
                                   maxPlanningTime_,
                                   &fullPlanFF);
                                                                                dprint1("here4");

    mlcore::State* currentState = s0;
    for (string actionName : fullPlanFF) {
        std::cerr << currentState << " " << actionName << " ";
        mlcore::Action* action = problem_->getActionFromName(actionName);
        std::cerr << action << std::endl;
        currentState =
            mostLikelyOutcome(problem_, currentState, action, true);
    }
    return problem_->getActionFromName(actionNameAndCost.first);

}

}
