#include "rational.h"
#include "exceptions.h"

inline int abs( int x )
{
  return( x < 0 ? -x : x );
}

static int
gcd( int n, int m )
{
  int a = abs( n );
  int b = abs( m );
  while( b > 0 )
    {
      int c = b;
      b = a % b;
      a = c;
    }
  return( a );
}

static int
lcm( int n, int m )
{
  return( n / gcd( n, m ) * m );
}

std::pair<int,int> 
Rational::multipliers( int n, int m )
{
  int f = lcm( n, m );
  return( std::make_pair( f/n, f/m ) );
}

Rational::Rational( int n, int m )
{
  if( m == 0 )
    throw Exception( "division by zero" );
  else
    {
      int d = gcd( n, m );
      numerator_ = n / d;
      denominator_ = m / d;
      if( denominator_ < 0 )
	{
	  numerator_ *= -1;
	  denominator_ *= -1;
	}
    }
}

Rational::Rational( const char* s )
  : numerator_(0)
{
  const char* si = s;
  for( ; *si != '\0' && *si != '.' && *si != '/'; ++si )
    numerator_ = 10 * numerator_ + (*si - '0');

  if( *si == '/' )
    {
      denominator_ = 0;
      for( ++si; *si != '\0'; ++si )
	denominator_ = 10*denominator_ + (*si - '0');
      if( denominator_ == 0 )
	throw Exception( "division by zero" );
      int d = gcd( numerator_, denominator_ );
      numerator_ /= d;
      denominator_ /= d;
    }
  else if( *si == '.' )
    {
      int a = numerator_;
      numerator_ = 0;
      denominator_ = 1;
      for( ++si; *si != '\0'; ++si )
	{
	  numerator_ = 10 * numerator_ + (*si - '0');
	  denominator_ *= 10;
	}
      int d = gcd( numerator_, denominator_ );
      numerator_ /= d;
      denominator_ /= d;
      numerator_ += a * denominator_;
    }
  else
    {
      denominator_ = 1;
    }
}

bool
operator<( const Rational& q, const Rational& p )
{
  std::pair<int,int> m = Rational::multipliers( q.denominator(), p.denominator() );
  return( q.numerator() * m.first < p.numerator() * m.second );
}

bool
operator<=( const Rational& q, const Rational& p )
{
  std::pair<int,int> m = Rational::multipliers( q.denominator(), p.denominator() );
  return( q.numerator() * m.first <= p.numerator() * m.second );
}

bool
operator==( const Rational& q, const Rational& p )
{
  std::pair<int,int> m = Rational::multipliers( q.denominator(), p.denominator() );
  return( q.numerator() * m.first == p.numerator() * m.second );
}

bool
operator!=( const Rational& q, const Rational& p )
{
  std::pair<int,int> m = Rational::multipliers( q.denominator(), p.denominator() );
  return( q.numerator() * m.first != p.numerator() * m.second );
}

bool
operator>=( const Rational& q, const Rational& p )
{
  std::pair<int,int> m = Rational::multipliers( q.denominator(), p.denominator() );
  return( q.numerator() * m.first >= p.numerator() * m.second );
}

bool
operator>( const Rational& q, const Rational& p )
{
  std::pair<int,int> m = Rational::multipliers( q.denominator(), p.denominator() );
  return( q.numerator() * m.first > p.numerator() * m.second );
}

Rational
operator+( const Rational& q, const Rational& p )
{
  std::pair<int,int> m = Rational::multipliers( q.denominator(), p.denominator() );
  return( Rational( q.numerator() * m.first + p.numerator() * m.second,
		    q.denominator() * m.first ) );
}

Rational
operator-( const Rational& q, const Rational& p )
{
  std::pair<int,int> m = Rational::multipliers( q.denominator(), p.denominator() );
  return( Rational( q.numerator() * m.first - p.numerator() * m.second,
		    q.denominator() * m.first ) );
}

Rational
operator*( const Rational& q, const Rational& p )
{
  int d1 = gcd( q.numerator(), p.denominator() );
  int d2 = gcd( p.numerator(), q.denominator() );
  return( Rational( (q.numerator()/d1) * (p.numerator()/d2),
		    (q.denominator()/d2) * (p.denominator()/d1) ) );
}

Rational
operator/( const Rational& q, const Rational& p )
{
  if( p == 0 ) throw Exception( "division by zero" );
  int d1 = gcd( q.numerator(), p.numerator() );
  int d2 = gcd( p.denominator(), q.denominator() );
  return( Rational( (q.numerator()/d1) * (p.denominator()/d2),
		    (q.denominator()/d2) * (p.numerator()/d1) ) );
}

std::ostream&
operator<<( std::ostream& os, const Rational& q )
{
  os << q.numerator();
  if( q.denominator() != 1 )
    os << '/' << q.denominator();
  return( os );
}
