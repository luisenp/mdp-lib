#ifndef MDPLIB_RATIONAL_H
#define MDPLIB_RATIONAL_H

#include <iostream>

class Rational
{
public:
    Rational() : num_(0), den_(1) { }

    Rational(int num) : num_(num), den_(1) { }

    Rational(int num, int den) : num_(num), den_(den) { }

    int num() const
    {
        return num_;
    }

    int den() const
    {
        return den_;
    }

    bool operator<(const Rational &other)
    {
        return num_*other.den_ < other.num_*den_;
    }

    friend std::ostream& operator<<(std::ostream& os, const Rational &r);

private:
    const int num_;
    const int den_;

};

Rational operator+(const Rational &r1, Rational &r2);

Rational operator-(const Rational &r1, Rational &r2);

Rational operator*(const Rational &r1, Rational &r2);

Rational operator/(const Rational &r1, Rational &r2);

#endif // MDPLIB_RATIONAL_H
