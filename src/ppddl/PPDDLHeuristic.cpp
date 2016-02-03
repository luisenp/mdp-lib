#include "../../include/ppddl/mini-gpt/heuristics.h"

#include "../../include/ppddl/PPDDLHeuristic.h"
#include "../../include/ppddl/ppddl_state.h"
#include "../../include/ppddl/ppddl_problem.h"

namespace mlppddl
{

PPDDLHeuristic::PPDDLHeuristic(Problem* problem, HeuristicType type, int m)
{
    if (type == atomMin1Forward) {
        pHeuristic_ =
            new atomMin1ForwardHeuristic_t(*(problem->pProblem()));
    } else if (type == atomMinMForward) {
        pHeuristic_ =
            new atomMinMForwardHeuristic_t(*(problem->pProblem()), m);
    } else if (type == atomMinMBackward) {
        pHeuristic_ =
            new atomMinMBackwardHeuristic_t(*(problem->pProblem()), m);
    } else if (type == FF) {
        pHeuristic_ =
            new ffHeuristic_t(*(problem->pProblem()));
    }
}

double PPDDLHeuristic::cost(const mlcore::State* s) const
{
    State* ppddlState = (State *) s;
    return pHeuristic_->value(*(ppddlState->pState()));
}

} // namespace mlppdl
