#include "global.h"
#include "actions.h"
#include "algorithms.h"
#include "exceptions.h"
#include "graph.h"
#include "hash.h"
#include "heuristics.h"
#include "queue.h"

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <values.h>
#include <iostream>
#include <sstream>


/*******************************************************************************
 *
 * codification for atom lists up to a fixed size
 *
 ******************************************************************************/

class codification_t
{
public:
  static unsigned f_p( unsigned w )
    {
      return( ((w+1)*(w+1) - (w+1)) >> 1 );
    }
  static unsigned f_w( unsigned z )
    {
      double t = (1 + sqrt( 1 + 8*z )) / 2;
      return( (unsigned)floor( t ) - 1 );
    }
  static unsigned f_x( unsigned z )
    {
      return( z - f_p( f_w( z ) ) );
    }
  static unsigned f_y( unsigned z )
    {
      return( f_p( f_w( z ) ) - z + f_w( z ) );
    }
  static unsigned code( unsigned x, unsigned y )
    {
      unsigned z = x*x + 2*x*y + y*y + 3*x + y;
      return( z >> 1 );
    }
  static unsigned code( const atomList_t &alist );
  static unsigned code( const ushort_t *array, size_t size );
  static void decode( unsigned ucode, atomList_t &alist );
};

inline unsigned
codification_t::code( const atomList_t &alist )
{
  assert( alist.size() < 8 );
  unsigned ucode = 0;
  if( alist.size() > 0 )
    {
      ucode = alist.atom( 0 );
      for( size_t i = 1; i < alist.size(); ++i )
	ucode = code( ucode, alist.atom( i ) );

      if( ((ucode<<3)>>3) != ucode )
	throw Exception( "precision overflow in codification_t::code(const atomList_t&)" );

      ucode = alist.size() + (ucode<<3);
    }
  return( ucode );
}

inline unsigned
codification_t::code( const ushort_t *array, size_t size )
{
  assert( size < 8 );
  unsigned ucode = 0;
  if( size > 0 )
    {
      ucode = array[0];
      for( size_t i = 1; i < size; ++i )
	ucode = code( ucode, array[i] );

      if( ((ucode<<3)>>3) != ucode )
	throw Exception( "precision overflow in codification_t::code(const ushort_t*,size_t)" );

      ucode = size + (ucode<<3);
    }
  return( ucode );
}

inline void
codification_t::decode( unsigned ucode, atomList_t &alist )
{
  alist.clear();
  size_t size = ucode & 0x7;
  ucode = ucode >> 3;
  if( size == 1 )
    alist.insert( ucode );
  else if( size > 1 )
    {
      for( size_t i = 0; i < size - 1; ++i )
	{
	  alist.insert( f_y( ucode ) );
	  ucode = f_x( ucode );
	}
      alist.insert( ucode );
    }
}


/*******************************************************************************
 *
 * zero heuristic
 *
 ******************************************************************************/

zeroHeuristic_t::zeroHeuristic_t( const problem_t &problem )
  : heuristic_t(problem)
{
  if( gpt::verbosity >= 500 )
    std::cout << "<zero>: new" << std::endl;
}

zeroHeuristic_t::~zeroHeuristic_t()
{
  if( gpt::verbosity >= 500 )
    std::cout << "<zero>: deleted" << std::endl;
}


/*******************************************************************************
 *
 * min-min-lrtdp heuristic
 *
 ******************************************************************************/

minMinLRTDPHeuristic_t::minMinLRTDPHeuristic_t( const problem_t &problem,
						heuristic_t &heur )
  : heuristic_t(problem), relaxation_(problem.weak_relaxation()), heur_(&heur)
{
  hash_table_ = new hash_t( gpt::initial_hash_size, *heur_ );
  algorithm_ = new lrtdp_t( relaxation_, *hash_table_, 0 );

  if( gpt::verbosity >= 500 )
    std::cout << "<min-min-lrtdp>: new" << std::endl;
}

minMinLRTDPHeuristic_t::~minMinLRTDPHeuristic_t()
{
  delete algorithm_;
  delete hash_table_;
  problem_t::unregister_use( &relaxation_ );

  if( gpt::verbosity >= 500 )
    std::cout << "<min-min-lrtdp>: deleted" << std::endl;
}

double
minMinLRTDPHeuristic_t::value( const state_t &state )
{
  double result = algorithm_->value( state );
  if( gpt::verbosity >= 450 )
    {
      std::cout << "<min-min-lrtdp>: heuristic for " << state  << " = " << result << std::endl;
    }
  return( result );
}

void
minMinLRTDPHeuristic_t::statistics( std::ostream &os ) const
{
}


/*******************************************************************************
 *
 * min-min-ida* heuristic
 *
 ******************************************************************************/

minMinIDAHeuristic_t::minMinIDAHeuristic_t( const problem_t &problem,
					    heuristic_t &heur, bool useTT )
  : heuristic_t(problem), relaxation_(problem.weak_relaxation()), heur_(&heur)
{
  hash_table_ = new hash_t( gpt::initial_hash_size, *heur_ );
  algorithm_ = new IDA_t( relaxation_, *hash_table_, useTT );

  if( gpt::verbosity >= 500 )
    std::cout << "<min-min-ida*>: new" << std::endl;
}

minMinIDAHeuristic_t::~minMinIDAHeuristic_t()
{
  delete algorithm_;
  delete hash_table_;
  problem_t::unregister_use( &relaxation_ );

  if( gpt::verbosity >= 500 )
    std::cout << "<min-min-ida*>: deleted" << std::endl;
}

double
minMinIDAHeuristic_t::value( const state_t &state )
{
  double result = algorithm_->value( state );
  if( gpt::verbosity >= 450 )
    {
      std::cout << "<min-min-ida*>: heuristic for " << state  << " = " << result << std::endl;
    }
  return( result );
}

void
minMinIDAHeuristic_t::statistics( std::ostream &os ) const
{
}


/*******************************************************************************
 *
 * dijkstra heuristic
 *
 ******************************************************************************/

dijkstraHeuristic_t::dijkstraHeuristic_t( const problem_t &problem,
						heuristic_t &heur, unsigned size )
  : heuristic_t(problem), relaxation_(problem.weak_relaxation()), heur_(&heur), size_(size)
{
  hash_table_ = new hash_t( gpt::initial_hash_size, *heur_ );
  algorithm_ = new boundedDijkstra_t( relaxation_, *hash_table_, size );

  if( gpt::verbosity >= 500 )
    std::cout << "<dijkstra-" << size_ << ">: new" << std::endl;
}

dijkstraHeuristic_t::~dijkstraHeuristic_t()
{
  delete algorithm_;
  delete hash_table_;
  problem_t::unregister_use( &relaxation_ );

  if( gpt::verbosity >= 500 )
    std::cout << "<dijkstra-" << size_ << ">: deleted" << std::endl;
}

double
dijkstraHeuristic_t::value( const state_t &state )
{
  double result = algorithm_->value( state );
  if( gpt::verbosity >= 450 )
    {
      std::cout << "<dijkstra-" << size_ << ">: heuristic for " << state  << " = " << result << std::endl;
    }
  return( result );
}

void
dijkstraHeuristic_t::statistics( std::ostream &os ) const
{
}


/*******************************************************************************
 *
 * atom-min-1-forward heuristic
 *
 ******************************************************************************/

atomMin1ForwardHeuristic_t::atomMin1ForwardHeuristic_t( const problem_t &problem )
  : heuristic_t(problem), relaxation_(problem.strong_relaxation())
{
  if( gpt::verbosity >= 500 )
    std::cout << "<atom-min-1-forward>: new" << std::endl;

  // generate random states and compute their heuristic
  if( gpt::xtra > 0 )
    {
      // recover one initial states
      const problem_t &relax = problem_.medium_relaxation();
      std::pair<state_t*,Rational> *initial = new std::pair<state_t*,Rational>[DISP_INT_SIZE];
      for( size_t i = 0; i < DISP_INT_SIZE; ++i )
	initial[i].first = new state_t;
      problem_.initial_states( initial );

      for( size_t i = 0; i < gpt::xtra; ++i )
	{
	  // set initial state
	  state_t state;
	  for( state_t::const_predicate_iterator ai = initial[0].first->predicate_begin(); ai != initial[0].first->predicate_end(); ++ai )
	    if( *ai % 2 == 0 )
	      state.add( *ai );

	  // apply random operators 
	  for( size_t steps = 0; steps < 1250; ++steps )
	    for( actionList_t::const_iterator ai = relax.actionsT().begin(); ai != relax.actionsT().end(); ++ai )
	      if( (*ai)->enabled( state ) && (drand48() > 0.5) )
		{
		  (*ai)->affect( state );
		  break;
		}

	  // output init description and value
	  std::cout << "  (:init";
	  for( state_t::const_predicate_iterator pi = state.predicate_begin(); pi != state.predicate_end(); ++pi )
	    {
	      std::cout << " ";
	      const Atom *atom = problem_t::atom_inv_hash_get( *pi );
	      if( !atom )
		std::cout << "(int-" << (*pi) << ")";
	      else
		problem_.print( std::cout, *atom );
	    }
	  std::cout << ")" << std::endl;
	  std::cout << "<state-value>: " << value( state ) << std::endl;
	}

      // clean
      for( size_t i = 0; i < DISP_INT_SIZE; ++i )
	delete initial[i].first;
      delete[] initial;
      exit( 0 );
    }
}

atomMin1ForwardHeuristic_t::~atomMin1ForwardHeuristic_t()
{
  problem_t::unregister_use( &relaxation_ );

  if( gpt::verbosity >= 500 )
    std::cout << "<atom-min-1-forward>: deleted" << std::endl;
}

double
atomMin1ForwardHeuristic_t::value( const state_t &state )
{
  actionList_t::const_iterator it;
  double result;
  state_t current( state );

  size_t steps = 0;
  bool change = true;
  relaxation_.complete_state( current );
  while ( change && !relaxation_.goalT().holds( current ) )
    {
      ++steps;
      change = false;
      for( it = relaxation_.actionsT().begin(); it != relaxation_.actionsT().end(); ++it )
	if( (*it)->enabled( current, relaxation_.nprec() ) )
	  {
	    change = (*it)->affect( current, relaxation_.nprec() ) || change;
	  }
    }

  if( relaxation_.goalT().holds( current, relaxation_.nprec() ) )
    result = (double)steps;
  else
    result = gpt::dead_end_value;

  if( gpt::verbosity >= 450 )
    {
      std::cout << "<atom-min-1-forward>: heuristic for " << state  << " = " << result << std::endl;
    }

  if( (result < gpt::dead_end_value) && problem_.goal_atom() )
    result = result - 1;
  if( result < gpt::dead_end_value )
    result = result * gpt::heuristic_weight;
  return( result );
}

void
atomMin1ForwardHeuristic_t::statistics( std::ostream &os ) const
{
}


/*******************************************************************************
 *
 * atom-min-<m>-forward heuristic
 *
 ******************************************************************************/

atomMinMForwardHeuristic_t::atomMinMForwardHeuristic_t( const problem_t &problem,
							size_t m )
  : heuristic_t(problem), m_(m), relaxation_(problem.medium_relaxation())
{
  array_ = (ushort_t*)malloc( m_ * sizeof(ushort_t) );

  if( gpt::verbosity >= 500 )
    std::cout << "<atom-min-" << m_ << "-forward>: new" << std::endl;
}

atomMinMForwardHeuristic_t::~atomMinMForwardHeuristic_t()
{
  free( array_ );
  problem_t::unregister_use( &relaxation_ );

  if( gpt::verbosity >= 500 )
    std::cout << "<atom-min-" << m_ << "-forward>: deleted" << std::endl;
}

double
atomMinMForwardHeuristic_t::value( const state_t &state )
{
  static atomList_t alist, slist;
  static actionList_t aset;
  static std::vector<unsigned> ssets, gssets;
  static priorityQueue_t<std::pair<unsigned,ushort_t>,pairINDEX> PQ;
  static std::map<unsigned,ushort_t> map;
  static std::map<unsigned,ushort_t>::const_iterator it;
  static state_t::const_predicate_iterator ai;

  // base case
  if( problem_.goalT().holds( state ) ) return( 0 );

  // initialize goal subsets
  gssets.clear();
  subsets( m_, relaxation_.goalT().atom_list( 0 ), 0, array_, 0, gssets );

  // transform state into atom list
  slist.clear();
  for( ai = state.predicate_begin(); ai != state.predicate_end(); ++ai )
    slist.insert( *ai );

  // insert initial m-subsets into priority queue
  map.clear();
  ssets.clear();
  subsets( m_, slist, 0, array_, 0, ssets );
  for( size_t i = 0; i < ssets.size(); ++i )
    {
      std::pair<unsigned,ushort_t> p( ssets[i], 0 | OPEN );
      PQ.push( p );
      map[p.first] = p.second;
    }

  // apply Dijkstra's to atom space
  size_t count = 0;
  while( !PQ.empty() )
    {
      std::pair<unsigned,ushort_t> p = PQ.top();
      PQ.pop();
      p.second = (p.second & 0xFF) | CLOSED;
      map[p.first] = p.second;
      codification_t::decode( p.first, alist );

      if( gpt::verbosity >= 500 )
	{
	  std::cout << "<atom-min-" << m_ << "-forward>: " << m_
		    << "-subset = " << p.first << alist
		    << ", value = " << (p.second & 0xFF) << std::endl;
	}

      // check if done
      for( size_t i = 0; i < gssets.size(); ++i )
	if( gssets[i] == p.first )
	  {
	    ++count;
	    break;
	  }
      if( count == gssets.size() ) break;

      // expand
      actionList_t::const_iterator ai = relaxation_.actionsT().begin();
      for( ; ai != relaxation_.actionsT().end(); ++ai )
	{
	  // check precondition
	  size_t i = 0;
	  assert( (*ai)->precondition().size() == 1 );
	  while( i < (*ai)->precondition().atom_list( 0 ).size() )
	    if( !alist.holds( (*ai)->precondition().atom_list( 0 ).atom( i++ ) ) )
	      break;
	  if( i < (*ai)->precondition().atom_list( 0 ).size() ) continue;

	  const deterministicAction_t *action = (const deterministicAction_t*)(*ai);
	  ushort_t cost = (p.second & 0xFF) + 1; // 1 should be (*ai)'s cost
	  atomList_t progress( alist );

	  // add add-list
	  for( i = 0; i < action->effect().s_effect().add_list().size(); ++i )
	    progress.insert( action->effect().s_effect().add_list().atom( i ) );

	  // remove del-list
	  for( i = 0; i < action->effect().s_effect().del_list().size(); ++i )
	    progress.remove( action->effect().s_effect().del_list().atom( i ) );

	  // insert m-subsets into priority queue
	  ssets.clear();
	  subsets( m_, progress, 0, array_, 0, ssets );
	  for( i = 0; i < ssets.size(); ++i )
	    {
	      it = map.find( ssets[i] );
	      if( (it == map.end()) ||
		  ((((*it).second & 0xFF00) == OPEN) && (cost < ((*it).second & 0xFF))) )
		{
		  if( it != map.end() ) PQ.remove( *it );
		  std::pair<unsigned,ushort_t> q( ssets[i], cost | OPEN );
		  PQ.push( q );
		  map[q.first] = q.second;
		}
	    }
	}
    }

  // compute value
  double result = 0;
  for( size_t i = 0; i < gssets.size(); ++i )
    if( (it = map.find( gssets[i] )) != map.end() )
      {
	result = MAX(result,((*it).second & 0xFF));
      }
    else
      {
	result = gpt::dead_end_value;
	break;
      }

  if( gpt::verbosity >= 450 )
    {
      std::cout << "<atom-min-" << m_ << "-forward>: heuristic for " << state  << " = " << result << std::endl;
    }

  if( (result < gpt::dead_end_value) && problem_.goal_atom() )
    result = result - 1;
  if( result < gpt::dead_end_value )
    result = result * gpt::heuristic_weight;
  return( result );
}

void
atomMinMForwardHeuristic_t::statistics( std::ostream &os ) const
{
}

void
atomMinMForwardHeuristic_t::subsets( size_t m, const atomList_t &alist,
				     size_t i, ushort_t *array, size_t j,
				     std::vector<unsigned> &ssets ) const
{
  if( (j == m) || (i == alist.size()) )
    {
      if( (j == m) || (j == alist.size()) )
	{
	  unsigned ucode = codification_t::code( array, j );
	  ssets.push_back( ucode );
	}
    }
  else
    {
      // insert ith atom into subset
      array[j] = alist.atom( i );
      subsets( m, alist, i+1, array, j+1, ssets );

      // skip ith atom
      subsets( m, alist, i+1, array, j, ssets );
    }
}


/*******************************************************************************
 *
 * atom-min-<m>-backward heuristic
 *
 ******************************************************************************/

#define _MTX(n,i,j)  (n*(i) + (j))
#define MTX(n,i,j)   _MTX(n,MIN(i,j),MAX(i,j))
#define LOW(n,m)     ((m)/n)
#define HIGH(n,m)    ((m)%n)

static const atomMinMBackwardHeuristic_t *heuristic_ = NULL;
static const problem_t *problem_ = NULL;
static const atomList_t *seeds_ = NULL;

static bool
is_cover( atomList_t &clique )
{
  static atomList_t orbit;

  orbit.clear();
  for( size_t i = 0; i < clique.size(); ++i )
    orbit.insert( clique.atom( i ) << 1 );

  return( !orbit.empty_intersection( *seeds_ ) && heuristic_->is_cover( *problem_, orbit ) );
}

void
atomMinMBackwardHeuristic_t::system_t::compute_fmask( void )
{
  fmask_.clear();
  for( size_t i = 0; i < focus().size(); ++i )
    fmask_.insert( focus().atom_list( i ) );
}

void
atomMinMBackwardHeuristic_t::system_t::compute_bmask( void )
{
  compute_fmask();
  bmask_.insert( fmask_ );
  for( size_t i = 0; i < base().size(); ++i )
    bmask_.insert( base().atom_list( i ) );
}

unsigned
atomMinMBackwardHeuristic_t::system_t::value( const atomList_t &alist ) const
{
  atomList_t proj = alist;
  proj.intersect( bmask() );

  // complete projection (if necessary)
  unsigned partial = gpt::dead_end_value;
  bool completion = false;
  for( size_t i = 0; i < base().size(); ++i )
    if( proj.empty_intersection( base().atom_list( i ) ) )
      {
	if( completion )
	  throw Exception( "sorry, more than one background orbit not yet supported" );

	for( size_t j = 0; j < base().atom_list(i).size(); ++j )
	  {
	    completion = true;
	    proj.insert( base().atom_list(i).atom( j ) );
	    unsigned val = value_aux( proj );
	    partial = MIN(partial,val);
	    proj.remove( base().atom_list(i).atom( j ) );
	  }
      }
  return( completion ? partial : value_aux( proj ) );
}

void
atomMinMBackwardHeuristic_t::system_t::print( std::ostream &os ) const
{
  os << "[ {";
  for( size_t i = 0; i < focus().size(); ++i )
    os << " " << focus().atom_list( i );
  os << " } {";
  for( size_t i = 0; i < base().size(); ++i )
    os << " " << base().atom_list( i );
  os << " } {";
  for( size_t i = 0; i < frame().size(); ++i )
    os << " " << frame().atom_list( i );
  os << " } ]";
}

atomMinMBackwardHeuristic_t::atomMinMBackwardHeuristic_t( const problem_t &problem, size_t m )
  : heuristic_t(problem), m_(m), relaxation_(problem.medium_relaxation())
{
  std::vector<std::pair<const atomList_t*,ushort_t> >::const_iterator ei;
  std::map<const std::string,system_t*>::iterator si;

  problem_t::no_more_atoms();
  generate_systems();
  compute_databases();

  // compute heuristic using Dijkstra's
  std::vector<std::pair<const atomList_t*,ushort_t> > expansion;
  priorityQueue_t<std::pair<const atomList_t*,ushort_t>,pairINDEX> PQ;
  for( si = systems_.begin(); si != systems_.end(); ++si )
    if( (*si).second->support() )
      {
	system_t *system = (*si).second;
	std::cout << "computing pattern database for " << (*si).first << " ... ";
	std::cout.flush();

	// seed is goal
	atomList_t state = relaxation_.goalT().atom_list( 0 );
	state.intersect( system->bmask() );

	// complete seed with base atoms and insert into PQ
	assert( PQ.empty() );
	bool completion = false;
	for( size_t i = 0; i < system->base().size(); ++i )
	  if( state.empty_intersection( system->base().atom_list( i ) ) )
	    {
	      if( completion )
		throw Exception( "sorry, more than one background orbit not yet supported" );

	      for( size_t j = 0; j < system->base().atom_list(i).size(); ++j )
		{
		  completion = true;
		  atomList_t *seed = new atomList_t( state );
		  seed->insert( system->base().atom_list(i).atom( j ) );
		  std::pair<const atomList_t*,ushort_t> p( seed, 0 | OPEN );
		  PQ.push( p );
		  system->hash().insert( p );
		}
	    }

	if( !completion )
	  {
	    const atomList_t *seed = new atomList_t( state );
	    std::pair<const atomList_t*,ushort_t> p( seed, 0 | OPEN );
	    PQ.push( p );
	    system->hash().insert( p );
	  }

	// apply Dijkstra's
	while( !PQ.empty() )
	  {
	    std::pair<const atomList_t*,ushort_t> p = PQ.top();
	    PQ.pop();
	    p.second = (p.second & 0xFF) | CLOSED;
	    hash_t::iterator it = system->hash().find( p.first );
	    (*it).second = p.second;

	    if( gpt::verbosity >= 500 )
	      {
		std::cout << "<atom-min-" << m_ << "-backward>: " << *p.first
			  << ", value = " << (p.second & 0xFF) << std::endl;
	      }

	    // expand
	    regression( *p.first, p.second & 0xFF, *system, expansion );
	    for( ei = expansion.begin(); ei != expansion.end(); ++ei )
	      {
		hash_t::iterator it = system->hash().find( (*ei).first );
		if( it == system->hash().end() )
		  {
		    std::pair<const atomList_t*,ushort_t> q( (*ei).first, (*ei).second | OPEN );
		    PQ.push( q );
		    system->hash().insert( q );
		  }
		else if( (((*it).second & 0xFF00) == OPEN) && ((*ei).second < ((*it).second & 0xFF)) )
		  {
		    PQ.remove( *it );
		    system->hash().erase( it, ++it );
		    std::pair<const atomList_t*,ushort_t> q( (*ei).first, (*ei).second | OPEN );
		    PQ.push( q );
		    system->hash().insert( q );
		  }
	      }
	  }

	std::cout << "size = " << system->hash().size() << std::endl;
      }

  // generate random states and compute their heuristic
  if( gpt::xtra > 0 )
    {
      // recover one initial states
      std::pair<state_t*,Rational> *initial = new std::pair<state_t*,Rational>[DISP_INT_SIZE];
      for( size_t i = 0; i < DISP_INT_SIZE; ++i )
	initial[i].first = new state_t;
      problem_.initial_states( initial );

      ::hash_t *htable = new ::hash_t( gpt::initial_hash_size, *this );
      algorithm_t *ida = new IDA_t( relaxation_, *htable, true );

      for( size_t i = 0; i < gpt::xtra; ++i )
	{
	  // set initial state
	  state_t state;
	  for( state_t::const_predicate_iterator ai = initial[0].first->predicate_begin(); ai != initial[0].first->predicate_end(); ++ai )
	    if( *ai % 2 == 0 )
	      state.add( *ai );

	  // apply random operators 
	  for( size_t steps = 0; steps < 1250; ++steps )
	    for( actionList_t::const_iterator ai = relaxation_.actionsT().begin(); ai != relaxation_.actionsT().end(); ++ai )
	      if( (*ai)->enabled( state ) && (drand48() > 0.5) )
		{
		  (*ai)->affect( state );
		  break;
		}

	  // output init description and value
	  std::cout << "  (:init";
	  for( state_t::const_predicate_iterator pi = state.predicate_begin(); pi != state.predicate_end(); ++pi )
	    {
	      std::cout << " ";
	      const Atom *atom = problem_t::atom_inv_hash_get( *pi );
	      if( !atom )
		std::cout << "(int-" << (*pi) << ")";
	      else
		problem_.print( std::cout, *atom );
	    }
	  std::cout << ")" << std::endl;
	  std::cout << "<state-value>: " << value( state ) << std::endl;
          std::cout << "<solution*>: " << ida->value( state ) << std::endl;
	}

      // clean
      for( size_t i = 0; i < DISP_INT_SIZE; ++i )
	delete initial[i].first;
      delete[] initial;
    }
}

atomMinMBackwardHeuristic_t::~atomMinMBackwardHeuristic_t()
{
  problem_t::unregister_use( &relaxation_ );

  for( std::map<const std::string,system_t*>::const_iterator si = systems_.begin(); si != systems_.end(); ++si )
    delete (*si).second;

  for( std::map<const std::string,std::pair<const atomList_t*,bool> >::const_iterator oi = orbits_.begin(); oi != orbits_.end(); ++oi )
    delete (*oi).second.first;

  if( gpt::verbosity >= 500 )
    std::cout << "<atom-min-" << m_ << "-backward>: deleted" << std::endl;
}

double
atomMinMBackwardHeuristic_t::value( const state_t &state )
{
  atomList_t alist;
  for( state_t::const_predicate_iterator pi = state.predicate_begin(); pi != state.predicate_end(); ++pi )
    alist.insert( *pi );

  unsigned result = 0;
  std::map<const std::string,std::vector<system_t*>*>::const_iterator dbi;
  for( dbi = databases_.begin(); dbi != databases_.end(); ++dbi )
    {
      unsigned val = value( *(*dbi).second, alist );
      result = MAX(result,val);
      if( result == gpt::dead_end_value ) break;
    }

  if( gpt::verbosity >= 450 )
    {
      std::cout << "<atom-min-" << m_ << "-backward>: heuristic for " << state << " = " << result << std::endl;
    }

  if( (result < gpt::dead_end_value) && problem_.goal_atom() )
    result = result - 1;
  if( result < gpt::dead_end_value )
    result = result * (unsigned)gpt::heuristic_weight;
  return( result );
}

void
atomMinMBackwardHeuristic_t::statistics( std::ostream &os ) const
{
}

unsigned
atomMinMBackwardHeuristic_t::value( const std::vector<system_t*> &database, const atomList_t &alist ) const
{
  unsigned result = 0;
  for( size_t i = 0; i < database.size(); ++i )
    {
      unsigned val = database[i]->value( alist );
      if( val == gpt::dead_end_value )
	return( gpt::dead_end_value );
      else
	result += val;
    }
  return( result );
}

bool
atomMinMBackwardHeuristic_t::is_cover( const problem_t &problem, const atomList_t &orbit ) const
{
  // check that if an atom from orbit is removed by an action, another is added
  for( actionList_t::const_iterator ai = problem.actionsT().begin(); ai != problem.actionsT().end(); ++ai )
    {
      const deterministicAction_t *a = (const deterministicAction_t*)(*ai);
      if( !orbit.empty_intersection( a->effect().s_effect().del_list() ) &&
	  orbit.empty_intersection( a->effect().s_effect().add_list() ) )
	return( false );
    }
  return( true );
}

bool
atomMinMBackwardHeuristic_t::prune( const atomList_t &alist, const system_t &system ) const
{
  for( size_t i = 0; i < system.frame().size(); ++i )
    if( alist.intersection_size( system.frame().atom_list( i ) ) > 1 )
      return( true );
  return( false );
}

void
atomMinMBackwardHeuristic_t::regression( const atomList_t &alist,
					 ushort_t cost,
					 const system_t &system,
					 std::vector<std::pair<const atomList_t*,ushort_t> > &expansion ) const
{
  // find all actions such that: (1) add(a) \cap alist != empty,
  //                             (2) del(a) \cap alist = empty
  //std::cout << "begin expansion of " << alist << std::endl;

  expansion.clear();
  atomList_t prec, add, del, result;
  for( actionList_t::const_iterator ai = relaxation_.actionsT().begin(); ai != relaxation_.actionsT().end(); ++ai )
    {
      const deterministicAction_t *action = (const deterministicAction_t*)(*ai);
      prec = action->precondition().atom_list( 0 );
      add = action->effect().s_effect().add_list();
      del = action->effect().s_effect().del_list();

      prec.intersect( system.bmask() );
      add.intersect( system.bmask() );
      del.intersect( system.bmask() );

      if( !alist.empty_intersection( add ) && alist.empty_intersection( del ) )
	{
	  result = alist;
	  result.insert( prec );
	  result.remove( add );

	  if( !prune( result, system ) )
	    {
	      ushort_t ncost = cost;
	      if( !add.empty_intersection( system.fmask() ) || !del.empty_intersection( system.fmask() ) )
		++ncost;

	      //std::cout << "  " << (*ai)->name() << ":" << result << ":" << ncost << std::endl;
	      std::pair<const atomList_t*,ushort_t> p( new atomList_t( result ), ncost );
	      expansion.push_back( p );
	    }
	}
    }

  //std::cout << "end" << std::endl;
}

void
atomMinMBackwardHeuristic_t::compute_mutexes( atomList_t &reachable, 
					      std::vector<unsigned> &mutexes,
					      std::pair<state_t*,Rational> *initial ) const
{
  actionList_t::const_iterator ai;

  // initialize bitmap
  unsigned natoms = problem_t::number_atoms();
  unsigned size = natoms * (natoms - 1);
  char *bitmap = (char*)calloc( size/8, sizeof(char) );

  // initialization
  std::vector<unsigned> initial_mutexes;
#if 1
  // method 1: include all pairs not in s_0
  for( size_t i = 0; i < reachable.size(); ++i )
    for( size_t j = i+1; j < reachable.size(); ++j )
      {
	bool found = false;
	ushort_t p = reachable.atom( i );
	ushort_t q = reachable.atom( j );
	for( size_t k = 0; initial[k].second != Rational( -1 ); ++k )
	  if( initial[k].first->holds(p) && initial[k].first->holds(q) )
	    {
	      found = true;
	      break;
	    }
	if( !found )
	  {
	    unsigned m = MTX(natoms,p,q);
	    initial_mutexes.push_back( m );
	    bitmap[m/8] = bitmap[m/8] | (1<<(m%8));
	  }
      }
#else
  // method 2: unions of two sets M_a and M_b.
  // M_a = initial_mutexes = pairs {p,q} not in s_0 such some action add p and deletes q
  for( ai = relaxation_.actionsT().begin(); ai != relaxation_.actionsT().end(); ++ai )
    {
      const deterministicAction_t *a = (const deterministicAction_t*)(*ai);
      for( size_t i = 0; i < a->effect().s_effect().add_list().size(); ++i )
	{
	  ushort_t p = a->effect().s_effect().add_list().atom( i );
	  if( (p % 2 == 0) && reachable.find(p) )
	    {
	      for( size_t j = 0; j < a->effect().s_effect().del_list().size(); ++j )
		{
		  ushort_t q = a->effect().s_effect().del_list().atom( j );
		  unsigned m = MTX(natoms,p,q);
		  if( (q % 2 == 0) && reachable.find(q) && !(bitmap[m/8] & (1<<(m%8))) )
		    {
		      bool found = false;
		      for( size_t k = 0; initial[k].second != Rational( -1 ); ++k )
			if( initial[k].first->holds(p) && initial[k].first->holds(q) )
			  {
			    found = true;
			    break;
			  }
		      if( !found )
			{
			  initial_mutexes.push_back( m );
			  bitmap[m/8] = bitmap[m/8] | (1<<(m%8));
			}
		    }
		}
	    }
	}
    }

#if 0
  // M_b = pairs {r,q} such that a {p,q} is in M_a and exists action with r in PREC and p in ADD
  std::vector<unsigned> mutexes_b;
  for( ai = relaxation_.actionsT().begin(); ai != relaxation_.actionsT().end(); ++ai )
    {
      const deterministicAction_t *a = (const deterministicAction_t*)(*ai);
      for( size_t i = 0; i < a->precondition().atom_list(0).size(); ++i )
	{
	  ushort_t r = a->precondition().atom_list(0).atom( i );
	  if( (r % 2 == 0) && reachable.find(r) )
	    {
	      for( size_t j = 0; j < a->effect().s_effect().add_list().size(); ++j )
		{
		  ushort_t p = a->effect().s_effect().add_list().atom( j );
		  if( (p % 2 == 0) && reachable.find(p) )
		    {
		      for( size_t k = 0; k < reachable.size(); ++k )
			{
			  unsigned q = reachable.atom( k );
			  unsigned m = MTX(natoms,p,q);
			  if( bitmap[m/8] & (1<<(m%8)) )
			    {
			      bool found = false;
			      for( size_t l = 0; initial[l].second != Rational( -1 ); ++l )
				if( initial[l].first->holds(r) && initial[l].first->holds(q) )
				  {
				    found = true;
				    break;
				  }
			      if( !found )
				{
				  m = MTX(natoms,r,q);
				  if( !(bitmap[m/8] & (1<<(m%8))) )
				    mutexes_b.push_back( m );
				}
			    }
			}
		    }
		}
	    }
	}
    }

  // join M_b into initial_mutexes
  for( size_t i = 0; i < mutexes_b.size(); ++i )
    {
      unsigned m = mutexes_b[i];
      if( !(bitmap[m/8] & (1<<(m%8))) )
	{
	  initial_mutexes.push_back( m );
	  bitmap[m/8] = bitmap[m/8] | (1<<(m%8));
	}
    }
#endif
#endif

  // initial mutexes
  if( gpt::verbosity >= 450 )
    {
      std::cout << "<initial-mutex-set>: begin" << std::endl;
      for( size_t i = 0; i < initial_mutexes.size(); ++i )
	{
	  unsigned mtx = initial_mutexes[i];
	  const Atom *atom = problem_t::atom_inv_hash_get( LOW(natoms,mtx) );
	  std::cout << "  mutex <" << LOW(natoms,mtx) << "," << HIGH(natoms,mtx) << "> = <";
	  if( !atom )
	    std::cout << "(int-" << LOW(natoms,mtx) << ")";
	  else
	    problem_.print( std::cout, *atom );
	  atom = problem_t::atom_inv_hash_get( HIGH(natoms,mtx) );
	  std::cout << ",";
	  if( !atom )
	    std::cout << "(int-" << HIGH(natoms,mtx) << ")";
	  else
	    problem_.print( std::cout, *atom );
	  std::cout << ">" << std::endl;
	}
      std::cout << "<initial-mutex-set>: end" << std::endl;
    }

  // refine initial set; i.e. remove all pairs that do not satisfy condition 2 
  // from [bonet,geffner. Planning as Heuristic Search.AIJ 129 (2001).]
  bool change = true;
  unsigned number = initial_mutexes.size();
  while( change )
    {
      change = false;
      std::cout << "<mutex-set>: size = " << number << std::endl;
      for( size_t i = 0; i < initial_mutexes.size(); ++i )
	{
	  unsigned mtx = initial_mutexes[i];
	  if( bitmap[mtx/8] & (1<<(mtx%8)) )
	    {
	      for( ai = relaxation_.actionsT().begin(); ai != relaxation_.actionsT().end(); ++ai )
		{
		  size_t j;
		  const deterministicAction_t *a = (const deterministicAction_t*)(*ai);
		  for( j = 0; j < a->precondition().atom_list( 0 ).size(); ++j )
		    if( (a->precondition().atom_list( 0 ).atom( j ) % 2 == 0) && 
			!reachable.find( a->precondition().atom_list( 0 ).atom( j ) ) )
		      break;
		  if( j == a->precondition().atom_list( 0 ).size() )
		    {
		      ushort_t q = 0;
		      bool check = false;
		      if( a->effect().s_effect().add_list().find( LOW(natoms,mtx) ) )
			{
			  check = true;
			  q = HIGH(natoms,mtx);
			}
		      else if( a->effect().s_effect().add_list().find( HIGH(natoms,mtx) ) )
			{
			  check = true;
			  q = LOW(natoms,mtx);
			}

		      if( check )
			{
			  if( a->effect().s_effect().del_list().find( q ) ) continue;
			  if( a->effect().s_effect().add_list().find( q ) ) break;
			  for( j = 0; j < a->precondition().atom_list( 0 ).size(); ++j )
			    if( a->precondition().atom_list(0).atom(j) % 2 == 0 )
			      {
				unsigned m = MTX( natoms, q, a->precondition().atom_list(0).atom(j) );
				if( bitmap[m/8] & (1<<(m%8)) ) break;
			      }
			  if( j == a->precondition().atom_list( 0 ).size() ) break;
			}
		    }
		}
	      
	      if( ai != relaxation_.actionsT().end() )
		{
		  change = true;
		  --number;
		  bitmap[mtx/8] = bitmap[mtx/8] & ~(1<<(mtx%8));
		}
	    }
	}
    }
  std::cout << "<mutex-set>: size = " << number << std::endl;

  // final mutex set
  for( size_t i = 0; i < initial_mutexes.size(); ++i )
    if( bitmap[initial_mutexes[i]/8] & (1<<(initial_mutexes[i]%8)) )
      mutexes.push_back( initial_mutexes[i] );

  if( gpt::verbosity >= 450 )
    {
      std::cout << "<mutex-set>: begin" << std::endl;
      for( size_t i = 0; i < mutexes.size(); ++i )
	{
	  unsigned mtx = mutexes[i];
	  const Atom *atom = problem_t::atom_inv_hash_get( LOW(natoms,mtx) );
	  std::cout << "  mutex <" << LOW(natoms,mtx) << "," << HIGH(natoms,mtx) << "> = <";
	  if( !atom )
	    std::cout << "(int-" << LOW(natoms,mtx) << ")";
	  else
	    problem_.print( std::cout, *atom );
	  atom = problem_t::atom_inv_hash_get( HIGH(natoms,mtx) );
	  std::cout << ",";
	  if( !atom )
	    std::cout << "(int-" << HIGH(natoms,mtx) << ")";
	  else
	    problem_.print( std::cout, *atom );
	  std::cout << ">" << std::endl;
	}
      std::cout << "<mutex-set>: end" << std::endl;
    }

  // cleanup
  free( bitmap );
}

void
atomMinMBackwardHeuristic_t::compute_orbits( void )
{
  atomList_t reachable;
  std::vector<unsigned> mutexes;

  // recover initial states
  std::pair<state_t*,Rational> *initial = new std::pair<state_t*,Rational>[DISP_INT_SIZE];
  for( size_t i = 0; i < DISP_INT_SIZE; ++i )
    initial[i].first = new state_t;
  problem_.initial_states( initial );

  // compute reachable atoms: start with initial states
  for( size_t i = 0; initial[i].second != Rational( -1 ); ++i )
    for( state_t::const_predicate_iterator ai = initial[i].first->predicate_begin(); ai != initial[i].first->predicate_end(); ++ai )
      if( *ai % 2 == 0 )
	reachable.insert( *ai );

  // compute reachable atoms: insert add lists of applicable operators
  size_t size = 1 + reachable.size();
  while( size != reachable.size() )
    {
      size = reachable.size();
      for( actionList_t::const_iterator ai = relaxation_.actionsT().begin(); ai != relaxation_.actionsT().end(); ++ai )
	{
	  size_t i;
	  const deterministicAction_t *a = (const deterministicAction_t*)(*ai);
	  for( i = 0; i < a->precondition().atom_list( 0 ).size(); ++i )
	    if( (a->precondition().atom_list( 0 ).atom( i ) % 2 == 0) && 
		!reachable.find( a->precondition().atom_list( 0 ).atom( i ) ) )
	      break;
	  if( i == a->precondition().atom_list( 0 ).size() )
	    for( size_t j = 0; j < a->effect().s_effect().add_list().size(); ++j )
	      if( a->effect().s_effect().add_list().atom( j ) % 2 == 0 )
		reachable.insert( a->effect().s_effect().add_list().atom( j ) );
	}
    }

  // compute mutexes
  compute_mutexes( reachable, mutexes, initial );

  // compute maximal subsets of pairwise-mutex atoms.
  // The seeds come from the initial states.
  atomList_t seeds, tmp;
  for( state_t::const_predicate_iterator ai = initial[0].first->predicate_begin(); ai != initial[0].first->predicate_end(); ++ai )
      if( *ai % 2 == 0 )
	seeds.insert( *ai );
  for( size_t i = 1; initial[i].second != Rational( -1 ); ++i )
    {
      tmp.clear();
      for( state_t::const_predicate_iterator ai = initial[i].first->predicate_begin(); ai != initial[i].first->predicate_end(); ++ai )
	if( *ai % 2 == 0 )
	  tmp.insert( *ai );
      seeds.intersect( tmp );
    }

  // clean initial
  for( size_t i = 0; i < DISP_INT_SIZE; ++i )
    delete initial[i].first;
  delete[] initial;

  // create mutex graph and compute maximal cliques
  unsigned natoms = problem_t::number_atoms();
  graph_t mutex_graph( natoms >> 1 );
  for( size_t i = 0; i < mutexes.size(); ++i )
    {
      unsigned mtx = mutexes[i];
      mutex_graph.add_edge( LOW(natoms,mtx) >> 1, HIGH(natoms,mtx) >> 1 );
    }

#if 0
  std::cout << "<mutex-graph>: begin" << std::endl;
  mutex_graph.print( std::cout );
  std::cout << "<mutex-graph>: end" << std::endl;
#endif

  ::seeds_ = &seeds;
  ::heuristic_ = this;
  ::problem_ = &relaxation_;
  std::vector<atomList_t*> cliques;
  //mutex_graph.maximal_cliques( cliques, UINT_MAX, &::is_cover );
  mutex_graph.maximal_disjoint_cliques( cliques, UINT_MAX, &::is_cover );
  for( size_t i = 0; i < cliques.size(); ++i )
    {
      atomList_t *orbit = new atomList_t;
      //xxxxxx std::cout << "<clique-" << 1+i << ">: {";
      for( size_t j = 0; j < cliques[i]->size(); ++j )
	{
	  ushort_t atm = cliques[i]->atom( j ) << 1;
	  orbit->insert( atm );
#if 0
	  const Atom *atom = problem_t::atom_inv_hash_get( atm );
	  std::cout << " " << atm << ":";
	  if( !atom )
	    std::cout << "(int-" << atm << ")";
	  else
	    problem_.print( std::cout, *atom );
#endif
	}
      //xxxxxxxx std::cout << " }" << std::endl;

      std::ostringstream ost;
      ost << "orbit-" << 1 + orbits_.size();
      bool goal = !relaxation_.goalT().atom_list(0).empty_intersection( *orbit );
      orbits_[ost.str()] = std::make_pair( orbit, goal );
      delete cliques[i];
    }
  ::seeds_ = NULL;
  ::heuristic_ = NULL;
  ::problem_ = NULL;
}

void
atomMinMBackwardHeuristic_t::compute_systems( void )
{
  std::vector<std::vector<const atomList_t*>*> focuses;
  std::map<const std::string,std::pair<const atomList_t*,bool> >::const_iterator oi;

  // create orbit graph
  std::vector<const atomList_t*> orbits;
  for( oi = orbits_.begin(); oi != orbits_.end(); ++oi )
    if( (*oi).second.second )
      orbits.push_back( (*oi).second.first );
  if( orbits.size() == 0 ) return;

  // compute m subsets of goal orbits
  std::vector<atomList_t*> subsets;
  graph_t orbit_graph( orbits.size() );
  orbit_graph.subsets( m_, subsets );
  for( size_t i = 0; i < subsets.size(); ++i )
    {
      std::vector<const atomList_t*> *focus = new std::vector<const atomList_t*>;
      for( size_t j = 0; j < subsets[i]->size(); ++j )
	focus->push_back( orbits[subsets[i]->atom(j)] );
      focuses.push_back( focus );
      delete subsets[i];
    }

  // pick 1 orbit for background
  const atomList_t *back_orbit = NULL;
  const std::string *back_name = NULL;
#if 0
  size_t relevance = 0;
  for( oi = orbits_.begin(); oi != orbits_.end(); ++oi )
    if( !(*oi).second.second )
      {
	for( actionList_t::const_iterator ai = relaxation_.actionsT().begin(); ai != relaxation_.actionsT().end(); ++ai )
	  {
	    size_t rel = 0;
	    const deterministicAction_t *a = (const deterministicAction_t*)(*ai);
	    rel += (*oi).second.first->intersection_size( a->effect().s_effect().add_list() );
	    rel += (*oi).second.first->intersection_size( a->effect().s_effect().del_list() );
	    if( !back_orbit || (rel > relevance) )
	      {
		relevance = rel;
		back_name = &(*oi).first;
		back_orbit = (*oi).second.first;
	      }
	  }
      }
#endif

  if( !back_orbit )
    std::cout << "<background>: <empty>" << std::endl;
  else
    std::cout << "<background>: " << *back_name << " = " << *back_orbit << std::endl;

  // generate systems
  for( size_t i = 0; i < focuses.size(); ++i )
    {
      system_t *system = new system_t;

      // focus
      for( size_t j = 0; j < focuses[i]->size(); ++j )
	system->focus_insert( (*focuses[i])[j] );

      // base
      if( back_orbit )
	system->base_insert( back_orbit );

      // frame (constraints)
      for( oi = orbits_.begin(); oi != orbits_.end(); ++oi )
	system->frame_insert( (*oi).second.first );

      system->set_goal( true );
      system->compute_bmask();

      std::ostringstream ost;
      ost << "system-" << 1 + systems_.size();
      systems_[ost.str()] = system;
      //xxxxxx std::cout << "<system>: " << ost.str() << " = " << *system << ", goal = " << system->goal() << std::endl;
    }
}

void
atomMinMBackwardHeuristic_t::generate_systems( void )
{
  if( (problem_.orbits().size() == 0) && (problem_.systems().size() == 0) )
    {
      compute_orbits();
      compute_systems();
    }
  else
    {
      // create and detect goal orbits
      for( std::map<const std::string,std::vector<const Atom*>*>::const_iterator oi = problem_.orbits().begin(); oi != problem_.orbits().end(); ++oi )
	{
	  bool goal = false;
	  atomList_t *orbit = new atomList_t;
	  for( std::vector<const Atom*>::const_iterator ai = (*oi).second->begin(); ai != (*oi).second->end(); ++ai )
	    {
	      ushort_t atom = problem_t::atom_hash_get( *(*ai) );
	      goal = relaxation_.goalT().atom_list( 0 ).holds( atom ) || goal;
	      orbit->insert( atom );
	    }
	  orbits_[(*oi).first] = std::make_pair( orbit, goal );
	  std::cout << "<orbit>: " << (*oi).first << " = " << *orbit << ", goal = " << goal << std::endl;
	}

      // create systems
      for( std::map<const std::string,const ::system_t*>::const_iterator si = problem_.systems().begin(); si != problem_.systems().end(); ++si )
	{
	  bool goal = true;
	  system_t *system = new system_t;
	  std::vector<const std::string*>::const_iterator ni;
	  
	  for( ni = (*si).second->focus().begin(); ni != (*si).second->focus().end(); ++ni )
	    {
	      std::map<const std::string,std::pair<const atomList_t*,bool> >::const_iterator oi = orbits_.find( *(*ni) );
	      if( oi == orbits_.end() ) throw Exception( "undefined orbit " + *(*ni) );
	      system->focus_insert( (atomList_t*)(*oi).second.first );
	      goal = (*oi).second.second && goal;
	    }

	  for( ni = (*si).second->base().begin(); ni != (*si).second->base().end(); ++ni )
	    {
	      std::map<const std::string,std::pair<const atomList_t*,bool> >::const_iterator oi = orbits_.find( *(*ni) );
	      if( oi == orbits_.end() ) throw Exception( "undefined orbit " + *(*ni) );
	      system->base_insert( (atomList_t*)(*oi).second.first );
	    }

	  for( ni = (*si).second->frame().begin(); ni != (*si).second->frame().end(); ++ni )
	    {
	      std::map<const std::string,std::pair<const atomList_t*,bool> >::const_iterator oi = orbits_.find( *(*ni) );
	      if( oi == orbits_.end() ) throw Exception( "undefined orbit " + *(*ni) );
	      system->frame_insert( (atomList_t*)(*oi).second.first );
	    }

	  system->set_goal( goal );
	  system->compute_bmask();
	  systems_[(*si).first] = system;
	  std::cout << "<system>: " << (*si).first << " = " << *system << ", goal = " << system->goal() << std::endl;
	}
    }
}

void
atomMinMBackwardHeuristic_t::compute_databases( void )
{
  std::map<const std::string,system_t*>::const_iterator si;
  if( systems_.size() == 0 ) return;

  // generate systems graph
  std::cout << "<systems>:";
  std::vector<std::pair<const std::string*,system_t*> > systems;
  for( si = systems_.begin(); si != systems_.end(); ++si )
    if( (*si).second->goal() )
      {
	systems.push_back( std::make_pair( &(*si).first, (*si).second ) );
	std::cout << " " << (*si).first;
      }
  std::cout << std::endl;

  atomList_t tmp1, tmp2;
  graph_t system_graph( systems.size() );
  for( size_t i = 0; i < systems.size(); ++i )
    {
      for( size_t k = 0; k < systems[i].second->focus().size(); ++k )
	tmp1.insert( systems[i].second->focus().atom_list( k ) );

      for( size_t j = i+1; j < systems.size(); ++j )
	{
	  for( size_t k = 0; k < systems[j].second->focus().size(); ++k )
	    tmp2.insert( systems[j].second->focus().atom_list( k ) );

	  if( tmp1.empty_intersection( tmp2 ) )
	    system_graph.add_edge( i, j );
	  tmp2.clear();
	}
      tmp1.clear();
    }

#if 0
  std::cout << "<system-graph>: begin" << std::endl;
  system_graph.print( std::cout );
  std::cout << "<system-graph>: end" << std::endl;
#endif

  // compute maximal cliques (disjoint pattern databases)
  std::vector<atomList_t*> cliques;
  system_graph.maximal_cliques( cliques, gpt::max_database_size );

  for( size_t i = 0; i < cliques.size(); ++i )
    {
      std::vector<system_t*> *db = new std::vector<system_t*>;
      std::cout << "<database-" << 1+i << ">: {";
      for( size_t j = 0; j < cliques[i]->size(); ++j )
        {
          db->push_back( systems[cliques[i]->atom(j)].second );
          systems[cliques[i]->atom(j)].second->set_support( true );
          std::cout << " " << *systems[cliques[i]->atom(j)].first;
        }
      std::cout << " }" << std::endl;

      std::ostringstream ost;
      ost << "db-" << 1+i;
      databases_[ost.str()] = db;
      delete cliques[i];
    }
}


/*******************************************************************************
 *
 * FF heuristic
 *
 ******************************************************************************/

ffHeuristic_t::ffHeuristic_t( const problem_t &problem )
  : heuristic_t(problem), relaxation_(problem.strong_relaxation())
{
  nprec_ = relaxation_.nprec();
  size_t n = problem_t::number_atoms();
  number_operators_ = relaxation_.actionsT().size();
  atom_idx_ = (uchar_t*)malloc( n * sizeof(uchar_t) );
  operator_idx_ = (uchar_t*)malloc( number_operators_ * sizeof(uchar_t) );
  operator_ctr_ = (uchar_t*)malloc( number_operators_ * sizeof(uchar_t) );
  operator_ptr_ = (const deterministicAction_t**)
    malloc( number_operators_ * sizeof(const deterministicAction_t*) );
  prec_ = new atomList_t[n];
  add_ = new atomList_t[n];
  goals_ = (atomList_t**)calloc( n, sizeof(atomList_t*) );
  true_ = (atomList_t**)calloc( n, sizeof(atomList_t*) );
  goals_[0] = new atomList_t;
  true_[0] = new atomList_t;

  size_t i = 0;
  actionList_t::const_iterator it;
  for( it = relaxation_.actionsT().begin(); it != relaxation_.actionsT().end(); ++it )
    {
      assert( (*it)->precondition().size() == 1 );
      operator_ptr_[i++] = (const deterministicAction_t*)(*it);
    }

  // set goal
  goal_ = new atomList_t;
  *goal_ = problem.goalT().atom_list( 0 );

  // set inverse prec/add table
  for( size_t o = 0; o < number_operators_; ++o )
    {
      for( size_t i = 0; i < operator_ptr_[o]->precondition().atom_list( 0 ).size(); ++i )
	{
	  ushort_t atom = operator_ptr_[o]->precondition().atom_list( 0 ).atom( i );
	  prec_[atom].insert( o );
	}
      for( size_t i = 0; i < operator_ptr_[o]->effect().s_effect().add_list().size(); ++i )
	{
	  ushort_t atom = operator_ptr_[o]->effect().s_effect().add_list().atom( i );
	  add_[atom].insert( o );
	}
    }

  if( gpt::verbosity >= 500 )
    std::cout << "<ff>: new" << std::endl;
}

ffHeuristic_t::~ffHeuristic_t()
{
  free( atom_idx_ );
  free( operator_idx_ );
  free( operator_ctr_ );
  free( operator_ptr_ );
  delete[] prec_;
  delete[] add_;

  for( ushort_t i = 0; i < problem_t::number_atoms(); ++i )
    {
      if( goals_[i] ) delete goals_[i];
      if( true_[i] ) delete true_[i];
    }
  free( goals_ );
  free( true_ );

  delete goal_;
  problem_t::unregister_use( &relaxation_ );

  if( gpt::verbosity >= 500 )
    std::cout << "<ff>: deleted" << std::endl;
}

unsigned
ffHeuristic_t::difficulty( ushort_t op ) const
{
  unsigned diff = 0;
  for( size_t i = 0; i < operator_ptr_[op]->precondition().atom_list( 0 ).size(); ++i )
    diff += atom_idx_[operator_ptr_[op]->precondition().atom_list( 0 ).atom( i )];
  return( diff );
}

ushort_t
ffHeuristic_t::select( ushort_t goal, size_t layer ) const
{
  unsigned min = 0;
  ushort_t minop = USHORT_MAX;
  bool found = false;
  for( size_t i = 0; i < add_[goal].size(); ++i )
    {
      ushort_t op = add_[goal].atom( i );
      if( (operator_idx_[op] == layer - 1) &&
	  (!found || (difficulty( op ) < min)) )
	{
	  found = true;
	  min = difficulty( op );
	  minop = op;
	}
    }
  return( minop );
}

void
ffHeuristic_t::schedule( ushort_t atom, int layer, atomList_t &schedule_ ) const
{
  if( atom_idx_[atom] == UCHAR_MAX )
    {
      atom_idx_[atom] = layer;
      if( goal_->holds( atom, nprec_ ) ) goals_[layer]->insert( atom );
      for( size_t i = 0; i < prec_[atom].size(); ++i )
	{
	  ushort_t op = prec_[atom].atom( i );
	  if( (operator_idx_[op] == UCHAR_MAX) &&
	      (operator_ctr_[op] < operator_ptr_[op]->precondition().atom_list( 0 ).size()) )
	    ++operator_ctr_[op];
	  if( operator_ctr_[op] == operator_ptr_[op]->precondition().atom_list( 0 ).size() )
	    {
	      operator_idx_[op] = layer;
	      schedule_.insert( op );
	    }
	}
    }
}

double
ffHeuristic_t::value( const state_t &state )
{
  static atomList_t schedule_, new_schedule_;
  static atomList_t plan;

  // base case
  if( problem_.goalT().holds( state, nprec_ ) ) return( 0 );

  // initialization
  for( ushort_t a = 0; a < problem_t::number_atoms(); ++a )
    atom_idx_[a] = UCHAR_MAX;
  for( size_t o = 0; o < number_operators_; ++o )
    {
      operator_ctr_[o] = 0;
      operator_idx_[o] = UCHAR_MAX;
    }
  for( ushort_t a = 0; a < problem_t::number_atoms(); ++a )
    {
      if( goals_[a] ) goals_[a]->clear();
      if( true_[a] ) true_[a]->clear();
    }

  // schedule initial actions (i.e, those applicable in state)
  schedule_.clear();
  for( ushort_t i = 0; i < problem_t::number_atoms(); i += 2 )
    if( state.holds( i ) )
      schedule( i, 0, schedule_ );
    else if( nprec_ )
      schedule( i+1, 0, schedule_ );

#if 0
  for( state_t::const_predicate_iterator ai = state.predicate_begin(); ai != state.predicate_end(); ++ai )
    schedule( *ai, 0, schedule_ );
#endif

  // check if goal achieved in state
  bool done = true;
  size_t layer = 0;
  for( size_t i = 0; done && (i < goal_->size()); ++i )
    done = done && (atom_idx_[goal_->atom(i)] == 0);

  // compute relaxed graphplan
  bool dead_end = false;
  while( (schedule_.size() > 0) && !done )
    {
      // allocate space
      ++layer;
      if( !goals_[layer] ) goals_[layer] = new atomList_t;
      if( !true_[layer] ) true_[layer] = new atomList_t;

      new_schedule_.clear();
      for( size_t i = 0; i < schedule_.size(); ++i )
	{
	  ushort_t op =  schedule_.atom( i );
	  const deterministicAction_t *act = operator_ptr_[op];
	  for( size_t j = 0; j < act->effect().s_effect().add_list().size(); ++j )
	    {
	      ushort_t atom = act->effect().s_effect().add_list().atom( j );
	      schedule( atom, layer, new_schedule_ );
	    }
	}
      schedule_ = new_schedule_;

      // check if all goal atoms have been established
      done = true;
      for( size_t i = 0; done && (i < goal_->size()); ++i )
	done = done && (atom_idx_[goal_->atom(i)] < UCHAR_MAX);
    }

  if( !done && (schedule_.size() == 0) )
    {
      dead_end = true;
      goto end;
    }

  if( gpt::verbosity >= 550 )
    {
      std::cout << "<ff>: goal = ";
      for( size_t i = 0; i < goal_->size(); ++i )
	std::cout << goal_->atom( i ) << " ";
      std::cout << std::endl;

      for( size_t i = 0; i <= layer; ++i )
	{
	  std::cout << "<ff>: fact layer " << i << " = ";
	  for( ushort_t j = 0; j < problem_t::number_atoms(); ++j )
	    if( atom_idx_[j] == i )
	      std::cout << j << " ";
	  std::cout << std::endl;

	  std::cout << "<ff>: oper layer " << i << " = ";
	  for( size_t j = 0; j < number_operators_; ++j )
	    if( operator_idx_[j] == i )
	      std::cout << j << " ";
	  std::cout << std::endl;
	}
    }

  // extract relaxed plan
  plan.clear();
  for( size_t i = layer; i > 0; --i )
    {
      assert( (goals_[i] != NULL) && (true_[i] != NULL) );
      for( size_t j = 0; j < goals_[i]->size(); ++j )
	{
	  ushort_t goal = goals_[i]->atom( j );
	  if( !true_[i]->holds( goal, nprec_ ) )
	    {
	      // select action at layer i-1 with min diff that achieves goal
	      ushort_t op = select( goal, i );
	      if( op == USHORT_MAX )
		{
		  dead_end = true;
		  goto end;
		}
	      plan.insert( op );
	      const deterministicAction_t *act = operator_ptr_[op];

	      // insert preconditions into appropriate layers
	      for( size_t k = 0; k < act->precondition().atom_list( 0 ).size(); ++k )
		{
		  ushort_t atom = act->precondition().atom_list( 0 ).atom( k );
		  if( (atom_idx_[atom] != 0) && !true_[i-1]->holds( atom, nprec_ ) )
		    goals_[atom_idx_[atom]]->insert( atom );
		}

	      // mark add-list as TRUE
	      for( size_t k = 0; k < act->effect().s_effect().add_list().size(); ++k )
		{
		  ushort_t atom = act->effect().s_effect().add_list().atom( k );
		  true_[i-1]->insert( atom );
		  true_[i]->insert( atom );
		}
	    }
	}
    }

 end:
  double result = (!dead_end ? plan.size() : gpt::dead_end_value);

  if( gpt::verbosity >= 450 )
    {
      std::cout << "<ff>: heuristic for " << state << " = " << result << std::endl;
    }

  // FF heuristic is size of relaxed plan
  if( (result < gpt::dead_end_value) && problem_.goal_atom() )
    result = result - 1;
  if( result < gpt::dead_end_value )
    result = result * gpt::heuristic_weight;
  return( result );
}

void
ffHeuristic_t::statistics( std::ostream &os ) const
{
}


/*******************************************************************************
 *
 * lookahead heuristic
 *
 ******************************************************************************/

lookaheadHeuristic_t::lookaheadHeuristic_t( const problem_t &problem,
					    heuristic_t &heur, unsigned depth )
  : heuristic_t(problem), heur_(heur), depth_(depth)
{
  display_ = new std::pair<state_t*,Rational>*[DISP_SIZE];
  for( size_t i = 0; i < DISP_SIZE; ++i )
    {
      display_[i] = new std::pair<state_t*,Rational>[DISP_INT_SIZE];
      for( size_t j = 0; j < DISP_INT_SIZE; ++j )
	display_[i][j].first = new state_t;
    }

  if( gpt::verbosity >= 500 )
    std::cout << "<lookahead-" << depth_ << ">: new" << std::endl;
}

lookaheadHeuristic_t::~lookaheadHeuristic_t()
{
  for( size_t i = 0; i < DISP_SIZE; ++i )
    {
      for( size_t j = 0; j < DISP_INT_SIZE; ++j )
	delete display_[i][j].first;
      delete[] display_[i];
    }
  delete[] display_;

  if( gpt::verbosity >= 500 )
    std::cout << "<lookahead-" << depth_ << ">: deleted" << std::endl;
}

double
lookaheadHeuristic_t::value( const state_t &state, unsigned depth )
{
  actionList_t::const_iterator ai;
  static int ddepth = -1;

  if( depth == 0 )
    return( heur_.value( state ) );

  if( problem_.goalT().holds( state ) ) 
    return( 0 );

  double min = 0;
  bool some = false;

  ++ddepth;
  for( ai = problem_.actionsT().begin(); ai != problem_.actionsT().end(); ++ai )
    if( (*ai)->enabled( state ) )
      {
	double qvalue = 0;
	problem_.expand( **ai, state, display_[ddepth] );
	for( size_t i = 0; display_[ddepth][i].second != Rational( -1 ); ++i )
	  qvalue += 
	    value(*display_[ddepth][i].first,depth-1) * display_[ddepth][i].second.double_value();
	qvalue = (*ai)->cost( state ) + qvalue;

	if( !some || (qvalue < min) )
	  {
	    some = true;
	    min = qvalue;
	  }
      }
  --ddepth;

  return( MIN(min,gpt::dead_end_value) );
}

double
lookaheadHeuristic_t::value( const state_t &state )
{
  double h = value( state, depth_ );

  if( gpt::verbosity >= 450 )
    {
      std::cout << "<lookahead-" << depth_ << ">: heuristic for ";
      state.print( std::cout );
      std::cout << " = " << h << std::endl;
    }

  return( h );
}

void
lookaheadHeuristic_t::statistics( std::ostream &os ) const
{
}
