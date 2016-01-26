#include "global.h"
#include "actions.h"
#include "problems.h"
#include "hash.h"

#include <math.h>
#include <limits.h>
#include <values.h>

static unsigned
prime( unsigned n )
{
  register unsigned i;
  static bool initialized = false;
  static unsigned p, current, size, *primes;
  
  if( !initialized )
    {
      p = 0;
      current = 2;
      size = gpt::initial_hash_size;
      primes = (unsigned*)calloc( size, sizeof(unsigned) );
      initialized = true;
    }

  do {
    unsigned bound = (unsigned)ceil( sqrt( (double)current ) );
    for( i = 0; (i < p) && (primes[i] < bound); ++i )
      if( current % primes[i] == 0 ) break;

    if( (i == p) || (primes[i] == bound) )
      {
	if( p == size )
	  {
	    size = size>>1;
	    primes = (unsigned*)realloc( primes, size * sizeof(unsigned) );
	  }
	primes[p++] = current;
	current += (p==1?1:2);
      }
    else
      current += 2;
  } while( primes[p-1] < n );

  return( primes[p-1] );
}


/*******************************************************************************
 *
 * hash
 *
 ******************************************************************************/

hash_t::hash_t( unsigned dimension, heuristic_t &heuristic )
{
  ext_ = false;
  size_ = 0;
  heuristic_ = &heuristic;
  dimension_ = prime( dimension );
  table_ = (hashEntry_t**)calloc( dimension_, sizeof(hashEntry_t*) );
  number_ = (unsigned*)calloc( dimension_, sizeof(unsigned) );

  display_ = new std::pair<state_t*,Rational>*[DISP_SIZE];
  for( size_t i = 0; i < DISP_SIZE; ++i )
    {
      display_[i] = new std::pair<state_t*,Rational>[DISP_INT_SIZE];
      for( size_t j = 0; j < DISP_INT_SIZE; ++j )
	display_[i][j].first = new state_t;
    }

  if( gpt::verbosity >= 300 )
    std::cout << "<hash>: new hash: dimension = " << dimension_ << std::endl;
}

hash_t::~hash_t()
{
  for( unsigned i = 0; i < dimension_; ++i )
    for( hashEntry_t *ptr = table_[i]; ptr != NULL; )
      {
	hashEntry_t *next = ptr->next_;
	delete ptr;
	ptr = next;
      }
  free( table_ );
  free( number_ );

  for( size_t i = 0; i < DISP_SIZE; ++i )
    {
      for( size_t j = 0; j < DISP_INT_SIZE; ++j )
	delete display_[i][j].first;
      delete[] display_[i];
    }
  delete[] display_;

  if( gpt::verbosity >= 300 )
    std::cout << "<hash>: deleted" << std::endl;
}

void
hash_t::rehash( void )
{
  unsigned tdimension = dimension_;
  dimension_ = prime( dimension_<<1 );
  hashEntry_t **ttable = table_;
  table_ = (hashEntry_t**)calloc( dimension_, sizeof(hashEntry_t*) );
  free( number_ );
  number_ = (unsigned*)calloc( dimension_, sizeof(unsigned) );

  if( gpt::verbosity >= 300 )
    {
      std::cout << "<hash>: rehash: size = " << size_ 
                << ", dimension = " << dimension_ << std::endl;
    }

  size_ = 0;
  hashEntry_t *ptr, *tmp;
  for( unsigned i = 0; i < tdimension; ++i )
    for( ptr = ttable[i]; ptr != NULL; ptr = tmp )
      {
	tmp = ptr->next_;
	insert( ptr );
      }

  free( ttable );
}

unsigned
hash_t::diameter( void ) const
{
  unsigned result = 0;
  for( unsigned i = 0; i < dimension_; ++i )
    result = (number_[i] > result ? number_[i] : result);
  return( result );
}

void
hash_t::print( std::ostream &os, const problem_t &problem ) const
{
  os << "<hash>: table begin" << std::endl;
  for( unsigned i = 0; i < dimension_; ++i )
    {
      if( table_[i] ) os << "  bucket[" << i << "] = { ";
      for( hashEntry_t *ptr = table_[i]; ptr != NULL; ptr = ptr->next_ )
	{
	  os << "(" << ptr << ":" << ptr->bits() << ":";
	  if( gpt::verbosity >= 450 )
	    ptr->state()->full_print( os, &problem );
	  else
	    os << ptr->state()->hash_value() << ":";
	  os << ":" << ptr->value() << ") ";
	}
      if( table_[i] ) os << "}" << std::endl;
    }
  os << "<hash>: table end" << std::endl;
}

void
hash_t::dump( std::ostream &os ) const
{
}

int
hash_t::bestAction( const state_t &state, const problem_t &problem,
		    double &val, const atomList_t *appliedActions )
{
  static int depth = -1;

  if( problem.goal().holds( state ) ) 
    {
      val = 0;
      for( size_t i = 0; i < problem.actionsT().size(); ++i )
	if( problem.actionsT()[i]->enabled( state ) ) return( i );
      return( -1 );
    }

  int mina = -1;
  double min = gpt::dead_end_value;
  bool some = false;

  ++depth;
  assert( depth < DISP_SIZE );
  for( size_t i = 0; i < problem.actionsT().size(); ++i )
    {
      if( problem.actionsT()[i]->enabled( state ) &&
	  (!appliedActions || appliedActions->empty_intersection( problem.restriction(i) )) )
	{
	  double qvalue = 0;
	  problem.expand( *problem.actionsT()[i], state, display_[depth] );
	  for( size_t j = 0; display_[depth][j].second != Rational( -1 ); ++j )
	    {
	      double v = value( *display_[depth][j].first );
	      qvalue += v * display_[depth][j].second.double_value();
	    }

	  qvalue = qvalue + problem.actionsT()[i]->cost( state );

	  if( !some || (qvalue < min) )
	    {
	      some = true;
	      min = qvalue;
	      mina = i;
	    }
	}
    }
  --depth;

  val = MIN(min,gpt::dead_end_value);
  return( mina );
}


/*******************************************************************************
 *
 * state hash
 *
 ******************************************************************************/

stateHash_t::stateHash_t( unsigned dimension )
{
  size_ = 0;
  dimension_ = prime( dimension );
  table_ = (stateHashEntry_t**)calloc( dimension_, sizeof(stateHashEntry_t*) );
  number_ = (unsigned*)calloc( dimension_, sizeof(unsigned) );

  if( gpt::verbosity >= 300 )
    std::cout << "<state-hash>: new hash: dimension = " << dimension_ << std::endl;
}

stateHash_t::~stateHash_t()
{
  for( unsigned i = 0; i < dimension_; ++i )
    for( stateHashEntry_t *ptr = table_[i]; ptr != NULL; )
      {
	stateHashEntry_t *next = ptr->next_;
	delete ptr;
	ptr = next;
      }
  free( table_ );
  free( number_ );

  if( gpt::verbosity >= 300 )
    std::cout << "<state-hash>: deleted" << std::endl;
}

void
stateHash_t::rehash( void )
{
  unsigned tdimension = dimension_;
  dimension_ = prime( dimension_<<1 );
  stateHashEntry_t **ttable = table_;
  table_ = (stateHashEntry_t**)calloc( dimension_, sizeof(stateHashEntry_t*) );
  free( number_ );
  number_ = (unsigned*)calloc( dimension_, sizeof(unsigned) );

  if( gpt::verbosity >= 300 )
    {
      std::cout << "<state-hash>: rehash: size = " << size_ 
                << ", dimension = " << dimension_ << std::endl;
    }

  size_ = 0;
  stateHashEntry_t *ptr, *tmp;
  for( unsigned i = 0; i < tdimension; ++i )
    for( ptr = ttable[i]; ptr != NULL; ptr = tmp )
      {
	tmp = ptr->next_;
	insert( ptr );
      }

  free( ttable );
}

unsigned
stateHash_t::diameter( void ) const
{
  unsigned result = 0;
  for( unsigned i = 0; i < dimension_; ++i )
    result = (number_[i] > result ? number_[i] : result);
  return( result );
}

void
stateHash_t::print( std::ostream &os, const problem_t &problem ) const
{
  os << "<state-hash>: table begin" << std::endl;
  for( unsigned i = 0; i < dimension_; ++i )
    {
      if( table_[i] ) os << "  bucket[" << i << "] = { ";
      for( stateHashEntry_t *ptr = table_[i]; ptr != NULL; ptr = ptr->next_ )
	os << "(" << ptr << ":" << ptr->state()->hash_value() << ") ";
      if( table_[i] ) os << "}" << std::endl;
    }
  os << "<state-hash>: table end" << std::endl;
}

void
stateHash_t::dump( std::ostream &os ) const
{
}


/*******************************************************************************
 *
 * atomlist hash
 *
 ******************************************************************************/

atomListHash_t::atomListHash_t( unsigned dimension )
{
  size_ = 0;
  dimension_ = prime( dimension );
  table_ = (atomListHashEntry_t**)calloc( dimension_, sizeof(atomListHashEntry_t*) );
  number_ = (unsigned*)calloc( dimension_, sizeof(unsigned) );

  if( gpt::verbosity >= 300 )
    std::cout << "<atomlist-hash>: new hash: dimension = " << dimension_ << std::endl;
}

atomListHash_t::~atomListHash_t()
{
  for( unsigned i = 0; i < dimension_; ++i )
    for( atomListHashEntry_t *ptr = table_[i]; ptr != NULL; )
      {
	atomListHashEntry_t *next = ptr->next_;
	delete ptr;
	ptr = next;
      }
  free( table_ );
  free( number_ );

  if( gpt::verbosity >= 300 )
    std::cout << "<atomlist-hash>: deleted" << std::endl;
}

void
atomListHash_t::rehash( void )
{
  unsigned tdimension = dimension_;
  dimension_ = prime( dimension_<<1 );
  atomListHashEntry_t **ttable = table_;
  table_ = (atomListHashEntry_t**)calloc( dimension_, sizeof(atomListHashEntry_t*) );
  free( number_ );
  number_ = (unsigned*)calloc( dimension_, sizeof(unsigned) );

  if( gpt::verbosity >= 300 )
    {
      std::cout << "<atomlist-hash>: rehash: size = " << size_ 
                << ", dimension = " << dimension_ << std::endl;
    }

  size_ = 0;
  atomListHashEntry_t *ptr, *tmp;
  for( unsigned i = 0; i < tdimension; ++i )
    for( ptr = ttable[i]; ptr != NULL; ptr = tmp )
      {
	tmp = ptr->next_;
	insert( ptr );
      }

  free( ttable );
}

unsigned
atomListHash_t::diameter( void ) const
{
  unsigned result = 0;
  for( unsigned i = 0; i < dimension_; ++i )
    result = (number_[i] > result ? number_[i] : result);
  return( result );
}

void
atomListHash_t::print( std::ostream &os, const problem_t &problem ) const
{
  os << "<atomlist-hash>: table begin" << std::endl;
  for( unsigned i = 0; i < dimension_; ++i )
    {
      if( table_[i] ) os << "  bucket[" << i << "] = { ";
      for( atomListHashEntry_t *ptr = table_[i]; ptr != NULL; ptr = ptr->next_ )
	os << "(" << ptr << ":" << ptr->alist()->hash_value() << ") ";
      if( table_[i] ) os << "}" << std::endl;
    }
  os << "<atomlist-hash>: table end" << std::endl;
}

void
atomListHash_t::dump( std::ostream &os ) const
{
}
