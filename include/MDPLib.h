#ifndef MDPLIB_MDPLIB_H
#define MDPLIB_MDPLIB_H

namespace mdplib
{
    extern double dead_end_cost;
    extern double epsilon;

    /* Bitmasks for state bits */
    const unsigned long VISITED = 1ul;
    const unsigned long SOLVED = 1ul<<1;
    const unsigned long CLOSED = 1ul<<2;
    const unsigned long VISITED_ASTAR = 1ul<<3;
    const unsigned long SOLVED_SSiPP = 1ul<<4;
    const unsigned long CLOSED_SSiPP = 1ul<<5;
}

#endif // MDPLIB_MDPLIB_H
