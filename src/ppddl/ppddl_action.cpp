#include "../../include/ppddl/ppddl_action.h"

namespace mlppddl
{

std::ostream& Action::print(std::ostream& os) const
{
    pAction_->print(os);
}

}
