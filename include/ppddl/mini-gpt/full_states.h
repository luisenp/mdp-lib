#ifndef PSTATES_H
#define PSTATES_H

#include "problems.h"
#include "rational.h"

#include <list>
#include <deque>
#include <map>

class Action;
class Application;
class Atom;
class AtomSet;
class State;
class ValueMap;

class hash_t;
class stateHash_t;
class hashEntry_t;


/*******************************************************************************
 *
 * state
 *
 ******************************************************************************/

class state_t
{
  typedef std::pair<ushort_t,Rational> pair_t;

  unsigned size_;
  unsigned digest_;
  char *data_;

  static stateHash_t *state_hash_;
  static bool state_space_generated_;
  static std::pair<state_t*,Rational> *display_;

  ushort_t* predicates( const char *data ) const { return( &((ushort_t*)data)[1] ); }
  pair_t* fluents( const char *data ) const
    {
      return( (pair_t*)(&((ushort_t*)data)[1+*(ushort_t*)data]) );
    }
  bool find_predicate( ushort_t p, ushort_t **ptr = NULL ) const;
  bool find_fluent( ushort_t f, pair_t **ptr = NULL ) const;
  void insert_predicate( ushort_t p, ushort_t *ptr );
  void insert_fluent( ushort_t f, Rational v, pair_t *ptr );
  void delete_predicate( unsigned count, ushort_t *ptr );
  void delete_fluent( unsigned count, pair_t *ptr );

public:
  explicit state_t();
  state_t( const State& state );
  state_t( const state_t& state );
  state_t( const problem_t& problem );
  ~state_t();

  static void initialize( const problem_t &problem );
  static void finalize( void );
  static const state_t* get_state( const state_t &state );
  static void generate_state_space( const problem_t &problem, hash_t &hash_table,
				    std::deque<hashEntry_t*> &space );

  size_t number_predicates( void ) const { return( *(ushort_t*)data_ ); }
  size_t number_fluents( void ) const
    {
      return( (size_ - (1 + number_predicates()) * sizeof(ushort_t)) / sizeof(pair_t) );
    }
  ushort_t predicate( size_t i ) const { return( predicates( data_ )[i] ); }
  pair_t fluent( size_t i ) const { return( fluents( data_ )[i] ); }
  unsigned size( void ) const { return( size_ ); }
  bool check( unsigned size, const char *data ) const;
  bool make_check( void ) const { return( check( size_, data_ ) ); }
  unsigned hash_value( void ) const { return( digest_ ); }
  unsigned digest( void ) const;
  void make_digest( void ) { digest_ = digest(); }
  bool operator==( const state_t &state ) const
    {
      return( (size_ == state.size_) &&
	      (digest_ == state.digest_) && 
	      !memcmp( data_, state.data_, size_ ) );
    }

  bool holds( ushort_t atom ) const { return( find_predicate( atom ) ); }
  bool holds( const Atom &atom ) const
    {
      return( holds( problem_t::atom_hash_get( atom ) ) );
    }
  bool holds( const Application &app, Rational value ) const;
  void add( ushort_t atom );
  void add( const Atom &atom ) { add( problem_t::atom_hash_get( atom ) ); }
  void add( const Application &app, Rational value );
  void clear( ushort_t atom );
  void clear( const Atom &atom ) { clear( problem_t::atom_hash_get( atom ) ); }

  const state_t& next( const Action *action ) const;
  double cost( const Action *action ) const { return( 1 ); }

  void print( std::ostream &os ) const;
  void full_print( std::ostream &os, const problem_t *problem ) const;
  void printXML( std::ostream &os, bool goal ) const;
  void send( std::ostream& os ) const;

public: // iterators
  class const_predicate_iterator
  {
    const state_t *state_;
    size_t idx_;

  protected:
    const_predicate_iterator( const state_t &st, size_t idx ) : state_(&st), idx_(idx) { }

  public:
    const_predicate_iterator() : state_(0), idx_(0) { } 
    ushort_t operator*( void ) const { return( state_->predicate( idx_ ) ); }
    const_predicate_iterator operator++( void ) // pre increment
      {
	++idx_;
	return( *this );
      }
    const_predicate_iterator operator++( int ) // post increment
      {
	const_predicate_iterator it( *this ); // use default copy const.
	++idx_;
	return( it );
      }
    bool operator==( const const_predicate_iterator &it ) const
      {
	return( (state_ == it.state_) && (idx_ == it.idx_) );
      }
    bool operator!=( const const_predicate_iterator &it ) const
      {
	return( (state_ != it.state_) || (idx_ != it.idx_) );
      }

    friend class state_t;
  };
  const const_predicate_iterator predicate_begin( void ) const
    {
      return( const_predicate_iterator( *this, 0 ) );
    }
  const const_predicate_iterator predicate_end( void ) const
    {
      return( const_predicate_iterator( *this, number_predicates() ) );
    }

  class const_fluent_iterator
  {
    const state_t *state_;
    size_t idx_;

  protected:
    const_fluent_iterator( const state_t &st, size_t idx ) : state_(&st), idx_(idx) { }

  public:
    const_fluent_iterator() : state_(0), idx_(0) { } 
    pair_t operator*( void ) const { return( state_->fluent( idx_ ) ); }
    const_fluent_iterator operator++( void ) // pre increment
      {
	++idx_;
	return( *this );
      }
    const_fluent_iterator operator++( int ) // post increment
      {
	const_fluent_iterator it( *this ); // use default copy const.
	++idx_;
	return( it );
      }
    bool operator==( const const_fluent_iterator &it ) const
      {
	return( (state_ == it.state_) && (idx_ == it.idx_) );
      }
    bool operator!=( const const_fluent_iterator &it ) const
      {
	return( (state_ != it.state_) || (idx_ != it.idx_) );
      }

    friend class state_t;
  };
  const const_fluent_iterator fluent_begin( void ) const
    {
      return( const_fluent_iterator( *this, 0 ) );
    }
  const const_fluent_iterator fluent_end( void ) const
    {
      return( const_fluent_iterator( *this, number_fluents() ) );
    }
};


/*******************************************************************************
 *
 * state/probability list
 *
 ******************************************************************************/

class stateProbList_t : public std::list<std::pair<state_t*,Rational> >
{
public:
  stateProbList_t() { }
  stateProbList_t( state_t *state, Rational p ) { push_back( std::make_pair(state,p) ); }
  stateProbList_t( const stateProbList_t &state_list )
    {
      for( stateProbList_t::const_iterator si = state_list.begin(); si != state_list.end(); ++si )
	{
	  state_t *state = new state_t( *(*si).first );
	  push_back( std::make_pair( state, (*si).second ) );
	}
    }
};

#endif // PSTATES_H
