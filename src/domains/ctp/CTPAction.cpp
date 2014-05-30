#include "CTPAction.h"

std::ostream& CTPAction::print(std::ostream& os) const
{
    os << "CTPAction: (" << from_ << "," << to_ << ")";
    return os;
}

int CTPAction::hash_value() const
{
    return from_ + 31*to_;
}
