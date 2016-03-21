#include "../../include/ppddl/mini-gpt/heuristics.h"

#include "../../include/ppddl/PPDDLHeuristic.h"
#include "../../include/ppddl/PPDDLProblem.h"
#include "../../include/ppddl/PPDDLState.h"

namespace mlppddl
{

PPDDLHeuristic::PPDDLHeuristic(PPDDLProblem* problem, HeuristicType type, int m)
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

double PPDDLHeuristic::cost(const mlcore::State* s)
{
    PPDDLState* ppddlState = (PPDDLState *) s;
    return pHeuristic_->value(*(ppddlState->pState()));
}

} // namespace mlppdl
