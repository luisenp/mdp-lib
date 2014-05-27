#include "../../include/util/rational.h"

#ifdef EXACT_COMP // Implementations for exact rational computation.

Rational operator+(Rational r1, Rational r2)
{
    return Rational(r1.num()*r2.den() + r1.den()*r2.num(), r1.den()*r2.den());
}

Rational operator-(Rational r1, Rational r2)
{
    return Rational(r1.num()*r2.den() - r1.den()*r2.num(), r1.den()*r2.den());
}

Rational operator*(Rational r1, Rational r2)
{
    return Rational(r1.num()*r2.num(), r1.den()*r2.den());
}

Rational operator/(Rational r1, Rational r2)
{
    return Rational(r1.num()*r2.den(), r1.den()*r2.num());
}

std::ostream& operator<<(std::ostream& os, Rational r)
{
    os << (double) r.num_ / r.den_;
    return os;
}

#else // Implementations using floating point arithmetic

Rational operator+(Rational r1, Rational r2)
{
    return Rational(r1.value() + r2.value());
}

Rational operator-(Rational r1, Rational r2)
{
    return Rational(r1.value() - r2.value());
}

Rational operator*(Rational r1, Rational r2)
{
    return Rational(r1.value() * r2.value());
}

Rational operator/(Rational r1, Rational r2)
{
    return Rational(r1.value() / r2.value());
}

std::ostream& operator<<(std::ostream& os, Rational r)
{
    os << r.value();
    return os;
}

#endif
