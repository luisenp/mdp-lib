#include "../../include/ppddl/PPDDLProblem.h"
#include "../../include/ppddl/PPDDLState.h"

namespace mlppddl
{
    std::ostream& State::print(std::ostream& os) const
    {
        pState_->full_print(os, ((Problem *) problem_)->pProblem());
    }
}
