#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "global.h"
#include "domains.h"
#include "expressions.h"
#include "formulas.h"
#include "functions.h"
#include "hash.h"
#include "problems.h"
#include "states.h"


/*******************************************************************************
 *
 * state
 *
 ******************************************************************************/

enum { CLEAR = 0x0, OPEN = 0x1, CLOSED = 0x2 };
stateHash_t* state_t::state_hash_ = 0;
std::pair<state_t*,Rational>* state_t::display_ = 0;
bool state_t::state_space_generated_ = false;
size_t state_t::size_ = 0;

void
state_t::initialize( const problem_t &problem )
{
  state_hash_ = new stateHash_t( gpt::initial_hash_size );
  size_ = (unsigned)ceil( (double)problem.number_atoms() / 32 );

  if( gpt::verbosity >= 300 )
    std::cout << "<state>: size = " << size_ << std::endl;

  display_ = new std::pair<state_t*,Rational>[DISP_INT_SIZE];
  for( size_t i = 0; i < DISP_INT_SIZE; ++i )
    display_[i].first = new state_t;
}

void
state_t::finalize( void )
{
  delete state_hash_;
  for( size_t i = 0; i < DISP_INT_SIZE; ++i )
    delete display_[i].first;
  delete[] display_;
}

void
state_t::statistics( std::ostream &os )
{
  os << "<state-hash>: number states = " << state_hash_->size() << std::endl;
}

const state_t*
state_t::get_state( const state_t &state )
{
  return( state_hash_->get( state )->state() );
}

void
state_t::generate_state_space( const problem_t &problem, hash_t &hash_table,
			       std::deque<hashEntry_t*> &space )
{
  actionList_t::const_iterator ai;
  std::deque<hashEntry_t*> Q;
  std::deque<hashEntry_t*>::iterator di;

  // check if state space has been already generated
  if( state_space_generated_ )
    {
      stateHash_t::const_iterator it = state_hash_->begin();
      while( it != state_hash_->end() )
	{
	  hashEntry_t *entry = hash_table.get( *(*it++) );
	  space.push_back( entry );
	}
      return;
    }

  // get initial states
  problem.initial_states( display_ );
  for( size_t i = 0; display_[i].second != Rational( -1 ); ++i )
    {
      hashEntry_t *entry = hash_table.get( *display_[i].first );
      entry->set_bits( OPEN );
      Q.push_front( entry );
    }

  // generate state space
  while( !Q.empty() )
    {
      hashEntry_t *tmp, *entry = Q.front();
      Q.pop_front();
      entry->set_bits( CLOSED );
      space.push_back( entry );

      if( gpt::verbosity >= 450 )
	{
	  std::cout << "SPACE: state ";
	  entry->state()->full_print( std::cout, &problem );
	  std::cout << std::endl;
	}

      for( ai = problem.actionsT().begin(); ai != problem.actionsT().end(); ++ai )
	if( (*ai)->enabled( *entry->state() ) )
	  {
	    problem.expand( **ai, *entry->state(), display_ );
	    for( size_t i = 0; display_[i].second != Rational( -1 ); ++i )
	      {
		if( !(tmp = hash_table.find( *display_[i].first  )) )
		  {
		    tmp = hash_table.insert( *display_[i].first );
		    tmp->set_bits( OPEN );
		    Q.push_front( tmp );
		  }
		else if( !(tmp->bits() & (OPEN|CLOSED)) )
		  {
		    tmp->set_bits( OPEN );
		    Q.push_front( tmp );
		  }
	      }
	  }
    }

  if( gpt::verbosity >= 450 )
    std::cout << "SPACE: <no-more>" << std::endl;

  // clear bits
  state_space_generated_ = true;
  for( di = space.begin(); di != space.end(); ++di )
    (*di)->set_bits( CLEAR );
}

state_t::state_t( const atomList_t &alist )
{
  notify( this, "state_t::state_t(const atomList_t&)" );
  data_ = (unsigned*)calloc( size_, sizeof(unsigned) );
  for( size_t i = 0; i < alist.size(); ++i )
    add( alist.atom( i ) );
}

state_t::state_t( const problem_t &problem )
{
  notify( this, "state_t::state_t(problem_t&)" );
  AtomSet::const_iterator ai;
  EffectList::const_iterator ei;
  AtomList::const_iterator li;
  AssignmentList::const_iterator si;

  for( ai = problem.init_atoms().begin(); ai != problem.init_atoms().end(); ++ai )
    if( !problem.domain().predicates().static_predicate( (*ai)->predicate() ) )
      add( **ai );

  for( ei = problem.init_effects().begin(); ei != problem.init_effects().end(); ++ei )
    {
      AtomList adds;
      AtomList deletes;
      AssignmentList assig;
      (*ei)->state_change( adds, deletes, assig, *this );

      for( li = adds.begin(); li != adds.end(); ++li )
	add( **li );

      for( si = assig.begin(); si != assig.end(); ++si )
	(*si)->affect( *this );
    }

  make_digest();
  assert( make_check() );
}

void
state_t::print( std::ostream &os ) const
{
  os << "[";
  for( state_t::const_predicate_iterator ai = predicate_begin(); ai != predicate_end(); ++ai )
    os << " " << *ai;
  os << " ]";
}

void
state_t::full_print( std::ostream &os, const problem_t *problem ) const
{
  state_t::const_predicate_iterator ai;

  os << "[";
  for( ai = predicate_begin(); ai != predicate_end(); ++ai )
    {
      os << " ";
      const Atom *atom = problem_t::atom_inv_hash_get( *ai );
      os << *ai << (*ai%2?":(not ":":");
      problem->print( os, *atom );
      if( *ai % 2 ) os << ")";
    }
  os << " ]";
}

void
state_t::printXML( std::ostream &os, bool goal ) const
{
  os << "<state>";
  if( goal ) os << "<is-goal/>";

  os << "</state>";
}

void 
state_t::send( std::ostream& os ) const
{
}
