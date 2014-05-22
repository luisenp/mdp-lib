#ifndef MDPLIB_RATIONAL_H
#define MDPLIB_RATIONAL_H

#include <iostream>

class Rational
{
private:
    long num_;
    long den_;

public:
    Rational() : num_(0), den_(1) { }

    Rational(long num) : num_(num), den_(1) { }

    Rational(long num, long den) : num_(num), den_(den) { }

    long num() const
    {
        return num_;
    }

    long den() const
    {
        return den_;
    }

    virtual Rational& operator=(const Rational& rhs)
    {
        if (this == &rhs)
            return *this;

        num_ =  rhs.num_;
        den_ =  rhs.den_;
        return *this;
    }

    bool operator==(Rational rhs) const
    {
        return num_ == rhs.num_ && den_ == rhs.den_;
    }

    bool operator<(Rational other)
    {
        return num_*other.den_ < other.num_*den_;
    }

    friend std::ostream& operator<<(std::ostream& os, Rational r);
};

Rational operator+(Rational r1, Rational r2);

Rational operator-(Rational r1, Rational r2);

Rational operator*(Rational r1, Rational r2);

Rational operator/(Rational r1, Rational r2);

#endif // MDPLIB_RATIONAL_H
