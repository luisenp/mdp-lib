#include "../include/action.h"
#include <iostream>

namespace mlcore
{
    std::ostream& operator<<(std::ostream& os, Action* a)
    {
        a->print(os);
    }
}
