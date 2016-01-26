#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "global.h"
#include "hash.h"
#include <assert.h>
#include <math.h>
#include <deque>
#include <set>

class action_t;
class hash_t;
class state_t;
class problem_t;
class heuristic_t;
class Action;


/*******************************************************************************
 *
 * algorithm (abstract class)
 *
 ******************************************************************************/

class algorithm_t
{
protected:
  const problem_t *problem_;

public:
  algorithm_t() : problem_(0) { }
  algorithm_t( const problem_t &problem ) : problem_(&problem) { }
  virtual ~algorithm_t() { }

  virtual const action_t *next( const state_t &state ) = 0;
  virtual double value( const state_t &state ) = 0;
  virtual void statistics( std::ostream &os ) const = 0;
};


/*******************************************************************************
 *
 * random
 *
 ******************************************************************************/

class random_t : public algorithm_t
{
public:
  random_t( const problem_t &problem ) : algorithm_t(problem) { }
  virtual ~random_t() { }

  virtual const action_t *next( const state_t &state )
    {
      std::set<const action_t*> actions;
      actionList_t::const_iterator ai;
      std::set<const action_t*>::const_iterator it;

      for( ai = problem_->actionsT().begin(); ai != problem_->actionsT().end(); ++ai )
	if( (*ai)->enabled( state ) )
	  actions.insert( *ai );

      assert( actions.size() > 0 );
      unsigned action = lrand48() % actions.size();
      for( it = actions.begin(); it != actions.end(); ++it )
	if( action-- == 0 ) return( *it );
      return( 0 );
    }
  virtual double value( const state_t &state ) { return( 0.0 ); }
  virtual void statistics( std::ostream &os ) const { }
};


/*******************************************************************************
 *
 * value iteration
 *
 ******************************************************************************/

class valueIteration_t : public algorithm_t
{
  hash_t *hash_table_;
  double epsilon_;
  enum { OPEN = 0x1, CLOSED = 0x2 };

  void solve( void );

public:
  valueIteration_t( const problem_t &problem, hash_t &hash_table, double epsilon );
  virtual ~valueIteration_t();

  virtual const action_t *next( const state_t &state )
    {
      double value;
      return( problem_->actionsT()[hash_table_->bestAction( state, *problem_, value )] );
    }
  virtual double value( const state_t &state )
    {
      hashEntry_t *entry = hash_table_->find( state );
      assert( !entry );
      return( entry->value() );
    }
  virtual void statistics( std::ostream &os ) const;
};


/*******************************************************************************
 *
 * lrtdp
 *
 ******************************************************************************/

class lrtdp_t : public algorithm_t
{
  hash_t *hash_table_;
  double epsilon_;
  std::pair<state_t*,Rational> *display_;
  enum { SOLVED = 0x1 };

  void solve( hashEntry_t *node ) { while( !(node->bits() & SOLVED) ) trial( node ); }
  void trial( hashEntry_t *node );
  bool checkSolved( hashEntry_t *node, std::deque<hashEntry_t*> &closed );

public:
  lrtdp_t( const problem_t &problem, hash_t &hash_table, double epsilon );
  virtual ~lrtdp_t();

  virtual const action_t *next( const state_t &state )
    {
      double value;
      hashEntry_t *node = hash_table_->get( state );
      if( !(node->bits() & SOLVED) ) solve( node );
      int action  = hash_table_->bestAction( state, *problem_, value );
      if( (action < 0) || (action >= (int)problem_->actionsT().size()) )
	return( NULL );
      else
	return( problem_->actionsT()[action] );
    }
  virtual double value( const state_t &state )
    {
      hashEntry_t *node = hash_table_->get( state );
      if( !(node->bits() & SOLVED) ) solve( node );
      return( node->value() );
    }
  virtual void statistics( std::ostream &os ) const;
};


/*******************************************************************************
 *
 * asp
 *
 ******************************************************************************/

class asp_t : public algorithm_t
{
  hash_t *hash_table_;
  unsigned simulations_;

  void trial( hashEntry_t *node );

public:
  asp_t( const problem_t &problem, hash_t &hash_table, unsigned simulations );
  virtual ~asp_t();

  virtual const action_t *next( const state_t &state )
    {
      double value;
      hashEntry_t *node = hash_table_->get( state );
      for( unsigned i = 0; i < simulations_; ++i )
	trial( node );

      unsigned action = hash_table_->bestAction( state, *problem_, value );
      node->update( value );
      return( problem_->actionsT()[action] );
    }
  virtual double value( const state_t &state )
    {
      hashEntry_t *node = hash_table_->get( state );
      for( unsigned i = 0; i < simulations_; ++i )
	trial( node );
      return( node->value() );
    }
  virtual void statistics( std::ostream &os ) const;
};


/*******************************************************************************
 *
 * hdp(0,j)
 *
 ******************************************************************************/

class hdpzj_t : public algorithm_t
{
  hash_t *hash_table_;
  double epsilon_;
  unsigned J_;
  double logbase_;

  bool dfs( hashEntryX_t *node, int cost, unsigned &index, hashEntryX_t* &top,
	    std::deque<hashEntryX_t*> &visited );
  void solve( hashEntryX_t* entry );

protected:
  enum { MARK = 0x1, VISITED = 0x2, ACTIVE = 0x4 };
  unsigned kappa( double f ) const { return( (unsigned)ceil( -log(f) / logbase_ ) ); }
  unsigned transition_kappa( const state_t &state, const action_t &action,
			     const state_t &nstate ) const;
  hashEntryX_t* get( const state_t &state ) 
    {
      return( (hashEntryX_t*)hash_table_->get( state ) );
    }

public:
  hdpzj_t( const problem_t &problem, hash_t &hash_table, double epsilon, unsigned J );
  virtual ~hdpzj_t();
  
  virtual const action_t *next( const state_t &state )
    {
      double value;
      return( problem_->actionsT()[hash_table_->bestAction( state, *problem_, value )] );
    }
  virtual double value( const state_t &state )
    {
      return( 0 );
    }
  virtual void statistics( std::ostream &os ) const;

  friend class plannerHDPZJ_t;
};


/*******************************************************************************
 *
 * IDA
 *
 ******************************************************************************/

class IDA_t : public algorithm_t
{
  hash_t *hash_table_;
  bool useTT_;
  enum { SOLVED = 0x1 };

  double ida( const state_t &state, double cost, double threshold, bool &found );
  double ida( hashEntry_t *node, double cost, double threshold, bool &found );
  double solve( const state_t &state );
  void solve( hashEntry_t *node );

public:
  IDA_t( const problem_t &problem, hash_t &hash_table, bool useTT );
  virtual ~IDA_t();

  virtual const action_t *next( const state_t &state )
    {
      return( 0 );
    }
  virtual double value( const state_t &state )
    {
      if( useTT_ )
	{
	  hashEntry_t *node = hash_table_->get( state );
	  if( !(node->bits() & SOLVED) )
	    solve( node );
	  return( node->value() );
	}
      else
	return( solve( state ) );
    }
  virtual void statistics( std::ostream &os ) const;
};


/*******************************************************************************
 *
 * bounded Dijkstra
 *
 ******************************************************************************/

class boundedDijkstra_t : public algorithm_t
{
  hash_t *hash_table_;
  unsigned size_;
  enum { OPEN = 0x100, CLOSED = 0x200 };

  double ida( hashEntry_t *node, double cost, double threshold, bool &found );
  void solve( hashEntry_t *node );

public:
  boundedDijkstra_t( const problem_t &problem, hash_t &hash_table, unsigned size );
  virtual ~boundedDijkstra_t();

  virtual const action_t *next( const state_t &state )
    {
      return( 0 );
    }
  virtual double value( const state_t &state )
    {
      hashEntry_t *node = hash_table_->get( state );
      if( !(node->bits() & CLOSED) )
	solve( node );
      return( node->value() );
    }
  virtual void statistics( std::ostream &os ) const;
};

#endif // ALGORITHMS_H
