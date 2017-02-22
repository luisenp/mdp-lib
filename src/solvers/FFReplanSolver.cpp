#include <vector>

#include "../../include/ppddl/PPDDLState.h"

#include "../../include/solvers/FFUtil.h"
#include "../../include/solvers/FFReplanSolver.h"

using namespace std;

namespace mlsolvers
{

mlcore::Action* FFReplanSolver::solve(mlcore::State* s0)
{
    startingPlanningTime_ = time(nullptr);

    if (s0->bestAction() != nullptr && !s0->deadEnd())
        return s0->bestAction();

    vector<string> fullPlan;
    s0->setBestAction(callFF(s0, fullPlan));
    if (s0->bestAction() == nullptr)
        s0->markDeadEnd();
    return s0->bestAction();
}


mlcore::Action*
FFReplanSolver::callFF(mlcore::State* s, vector<string>& fullPlan) const
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

    return problem_->getActionFromName(fullPlan[0]);
}

}
