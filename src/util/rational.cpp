#include "../../include/util/rational.h"

Rational operator+(const Rational& r1, Rational& r2)
{
    return Rational(r1.num()*r2.den() + r1.den()*r2.num(), r1.den()*r2.den());
}

Rational operator-(const Rational& r1, Rational& r2)
{
    return Rational(r1.num()*r2.den() - r1.den()*r2.num(), r1.den()*r2.den());
}

Rational operator*(const Rational& r1, Rational& r2)
{
    return Rational(r1.num()*r2.num(), r1.den()*r2.den());
}

Rational operator/(const Rational& r1, Rational& r2)
{
    return Rational(r1.num()*r2.den(), r1.den()*r2.num());
}

std::ostream& operator<<(std::ostream& os, const Rational& r)
{
    os << r.num_ << "/" << r.den_;
    return os;
}
