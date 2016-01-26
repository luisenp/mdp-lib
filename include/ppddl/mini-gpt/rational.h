#ifndef RATIONAL_H
#define RATIONAL_H

#include <iostream>
#include <utility>

class Rational
{
  int numerator_;
  int denominator_;

public:
  Rational( int n = 0 ) : numerator_(n), denominator_(1) { }
  Rational( int n, int m );
  Rational( const char* s );

  static std::pair<int, int> multipliers( int n, int m );

  int numerator( void ) const { return( numerator_ ); }
  int denominator( void ) const { return( denominator_ ); }
  double double_value( void ) const { return( (double)numerator()/denominator() ); }
  Rational abs( void ) const
    {
      if( numerator_ < 0 )
	return( Rational( -numerator_, denominator_ ) );
      else
	return( *this );
    }
};

bool operator<( const Rational& q, const Rational& p );
bool operator<=( const Rational& q, const Rational& p );
bool operator==( const Rational& q, const Rational& p );
bool operator!=( const Rational& q, const Rational& p );
bool operator>=( const Rational& q, const Rational& p );
bool operator>( const Rational& q, const Rational& p );
Rational operator+( const Rational& q, const Rational& p );
Rational operator-( const Rational& q, const Rational& p );
Rational operator*( const Rational& q, const Rational& p );
Rational operator/( const Rational& q, const Rational& p );

std::ostream& operator<<( std::ostream& os, const Rational& q );

#endif // RATIONAL_H
