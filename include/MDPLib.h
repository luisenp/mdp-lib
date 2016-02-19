#ifndef MDPLIB_MDPLIB_H
#define MDPLIB_MDPLIB_H

namespace mdplib
{
    extern double dead_end_cost;
    extern double epsilon;

    /* Bitmasks for state bits */
    const unsigned long VISITED = 1;
    const unsigned long SOLVED = 2;
    const unsigned long CLOSED = 4;
    const unsigned long VISITED_ASTAR = 8;
}

#endif // MDPLIB_MDPLIB_H
