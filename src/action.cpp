#include "../include/action.h"
#include <iostream>

std::ostream& operator<<(std::ostream& os, Action* a)
{
    a->print(os);
}

