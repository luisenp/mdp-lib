#include "../../include/ppddl/PPDDLAction.h"

namespace mlppddl
{

std::ostream& PPDDLAction::print(std::ostream& os) const
{
    pAction_->print(os);
}

}
