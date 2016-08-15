#include "../include/Action.h"
#include <iostream>

namespace mlcore
{
    std::ostream& operator<<(std::ostream& os, Action* a)
    {
        return a->print(os);
    }
}
