#ifndef MDPLIB_RATIONAL_H
#define MDPLIB_RATIONAL_H

#include <iostream>

#include "../mdplib.h"

#ifdef EXACT_COMP // Definition for exact rational computation.

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

    double value() const
    {
        return (double) num_ / den_;
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

#else // Definition using floating point arithmetic

#include <cmath>

class Rational
{
private:
    double value_;

public:
    Rational() : value_(0) { }

    Rational(double value) : value_(value) { }

    Rational(long num, long den)
    {
        value_ = (double) num / den;
    }

    double value() const
    {
        return value_;
    }

    virtual Rational& operator=(const Rational& rhs)
    {
        if (this == &rhs)
            return *this;
        value_ =  rhs.value_;
        return *this;
    }

    // Comparison implementations taken from http://stackoverflow.com/a/253874
    bool operator==(Rational rhs) const
    {
        if (value_ == rhs.value_)
            return true;

        double a = fabs(value_);
        double b = fabs(rhs.value_);
        double m = a < b ? a : b;
        return fabs(value_ - rhs.value_) <= (m * mdplib::epsilon);
    }

    bool operator>(Rational rhs)
    {
        double a = fabs(value_);
        double b = fabs(rhs.value_);
        double m = a < b ? b : a;

        return (value_ - rhs.value_) > (m * mdplib::epsilon);
    }

    bool operator<(Rational rhs)
    {
        double a = fabs(value_);
        double b = fabs(rhs.value_);
        double m = a < b ? b : a;

        return (rhs.value_ - value_) > (m * mdplib::epsilon);
    }

    friend std::ostream& operator<<(std::ostream& os, Rational r);
};

Rational operator+(Rational r1, Rational r2);

Rational operator-(Rational r1, Rational r2);

Rational operator*(Rational r1, Rational r2);

Rational operator/(Rational r1, Rational r2);

#endif // EXACT_COMP

#endif // MDPLIB_RATIONAL_H
