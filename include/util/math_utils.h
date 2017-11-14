#ifndef MDPLIB_MATH_H
#define MDPLIB_MATH_H

#include <cmath>

#include "../MDPLib.h"

namespace mdplib_math
{

bool equal(const double a, const double b) {
    return fabs(a - b) < mdplib::epsilon;
}

bool greaterThan(const double a, const double b) {
    return a > b - mdplib::epsilon;
}

bool lessThan(const double a, const double b) {
    return a < b + mdplib::epsilon;
}

bool lessThanOrEqual(const double a, const double b) {
    return  lessThan(a, b) || equal(a, b);
}

bool greaterThanOrEqual(const double a, const double b) {
    return  greaterThanOrEqual(a, b) || equal(a, b);
}

}

#endif // MDPLIB_MATH_H
