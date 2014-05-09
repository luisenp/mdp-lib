#include "../include/state.h"
#include <iostream>

std::ostream& operator<<(std::ostream& os, const State* s)
{
    s->print(os);
}
