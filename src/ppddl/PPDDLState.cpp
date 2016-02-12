#include "../../include/ppddl/PPDDLProblem.h"
#include "../../include/ppddl/PPDDLState.h"

namespace mlppddl
{
    std::ostream& PPDDLState::print(std::ostream& os) const
    {
        pState_->full_print(os, ((PPDDLProblem *) problem_)->pProblem());
    }
}
