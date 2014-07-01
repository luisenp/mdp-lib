#include "../../../include/domains/ctp/CTPAction.h"

std::ostream& CTPAction::print(std::ostream& os) const
{
    os << "CTPAction: (" << from_ << "," << to_ << ")";
    return os;
}

int CTPAction::hashValue() const
{
    return from_ + 31*to_;
}
