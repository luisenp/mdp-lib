#include "../include/state.h"
#include <iostream>

std::ostream& operator<<(std::ostream& os, State* s)
{
    s->print(os);
}
