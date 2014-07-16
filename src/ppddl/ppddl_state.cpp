#include "../../include/ppddl/ppddl_state.h"
#include "../../include/ppddl/ppddl_problem.h"

namespace mlppddl
{
    std::ostream& State::print(std::ostream& os) const
    {
        pState_->full_print(os, ((Problem *) problem_)->pProblem());
    }
}
