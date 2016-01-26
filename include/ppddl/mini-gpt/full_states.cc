#include <stdlib.h>
#include <assert.h>

#include "global.h"
#include "domains.h"
#include "expressions.h"
#include "formulas.h"
#include "functions.h"
#include "hash.h"
#include "problems.h"
#include "states.h"

extern "C" {
#include "md4.h"
};


/*******************************************************************************
 *
 * state
 *
 ******************************************************************************/

enum { CLEAR = 0x0, OPEN = 0x1, CLOSED = 0x2 };
stateHash_t *state_t::state_hash_ = 0;
std::pair<state_t*,Rational>* state_t::display_ = 0;
bool state_t::state_space_generated_ = false;

void
state_t::initialize( const problem_t &problem )
{
  state_hash_ = new stateHash_t( gpt::initial_hash_size );
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
  delete display_;
}

const state_t*
state_t::get_state( const state_t &state )
{
  return( state_hash_->get( state )->state() );
}

void
state_t::generate_state_space( const problem_t &problem, hash_t &hash_table, std::deque<hashEntry_t*> &space )
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
	std::cout << "SPACE: state (" << entry << ":"
		  << entry->state()->digest() << ")" << std::endl;

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

state_t::state_t()
{
  notify( this, "state_t::state_t()" );
  size_ = sizeof(ushort_t);
  data_ = (char*)malloc( sizeof(ushort_t) );
  ((ushort_t*)data_)[0] = 0;
  make_digest();
}

state_t::state_t( const state_t &state )
{
  notify( this, "state_t::state_t(state_t&)" );
  size_ = state.size_;
  digest_ = state.digest_;
  data_ = (char*)malloc( size_ );
  memcpy( data_, state.data_, size_ );
}

state_t::state_t( const problem_t &problem )
{
  notify( this, "state_t::state_t(problem_t&)" );
  AtomSet::const_iterator ai;
  ValueMap::const_iterator vi;
  EffectList::const_iterator ei;
  AtomList::const_iterator li;
  AssignmentList::const_iterator si;

  for( ai = problem.init_atoms().begin(); ai != problem.init_atoms().end(); ++ai )
    if( !problem.domain().predicates().static_predicate( (*ai)->predicate() ) )
      add( **ai );

  for( vi = problem.init_fluents().begin(); vi != problem.init_fluents().end(); ++vi )
    {
      Function function = (*vi).first->function();
      if( !problem.domain().functions().static_function( function ) )
	add( *(*vi).first, (*vi).second );
    }

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

state_t::~state_t()
{
  if( size_ > 0 ) free( data_ );
}

bool
state_t::find_predicate( ushort_t p, ushort_t **ptr ) const
{
  if( ((ushort_t*)data_)[0] == 0 )
    {
      if( ptr ) *ptr = (ushort_t*)&data_[size_];
      return( false );
    }
  else
    {
      register ushort_t *first_ = predicates( data_ );
      register ushort_t *last_ = (ushort_t*)fluents( data_ );
      register ushort_t *middle_ = first_ + (last_ - first_) / 2;

      while( (first_ < last_) && (*middle_ != p) )
	if( first_ == middle_ )
	  {
	    if( *middle_ < p ) ++middle_;
	    break;
	  }
	else
	  {
	    first_ = (*middle_<p?middle_:first_);
	    last_ = (*middle_<p?last_:middle_);
	    middle_ = first_ + (last_ - first_) / 2;
	  }

      if( ptr ) *ptr = middle_;
      return( (middle_ < (ushort_t*)fluents(data_)) && (*middle_ == p) );
    }
}

void
state_t::insert_predicate( ushort_t p, ushort_t *ptr )
{
  unsigned nsize_ = size_ + sizeof(ushort_t);
  char *ndata_ = (char*)malloc( nsize_ );
  ((ushort_t*)ndata_)[0] = ((ushort_t*)data_)[0] + 1;
  ptr = (!ptr?&((ushort_t*)data_)[1]:ptr);

  // copy data
  ushort_t *base_ = predicates( data_ );
  ushort_t *nbase_ = predicates( ndata_ );
  memcpy( nbase_, base_, (ptr - base_) * sizeof(ushort_t) );
  nbase_[ptr-base_] = p;
  memcpy( &nbase_[ptr-base_+1],
	  &base_[ptr-base_],
	  size_ - (1 + (ptr - base_)) * sizeof(ushort_t) );
  assert( check( nsize_, ndata_ ) );

  // set fields
  free( data_ );
  size_ = nsize_;
  data_ = ndata_;
}

void
state_t::delete_predicate( unsigned count, ushort_t *ptr )
{
  if( count > 0 )
    {
      unsigned nsize_ = size_ - count * sizeof(ushort_t);
      char *ndata_ = (char*)malloc( nsize_ );
      ((ushort_t*)ndata_)[0] = ((ushort_t*)data_)[0] - count;

      // copy predicates and rest
      ushort_t *base_ = predicates( data_ );
      ushort_t *nbase_ = predicates( ndata_ );
      memcpy( nbase_, base_, (ptr - base_) * sizeof(ushort_t) );
      memcpy( &nbase_[ptr - base_],
	      &base_[count + (ptr - base_)],
	      nsize_ - sizeof(ushort_t) - (ptr - base_) * sizeof(ushort_t) );
      assert( check( nsize_, ndata_ ) );

      // set fields
      free( data_ );
      size_ = nsize_;
      data_ = ndata_;
    }
}

bool
state_t::find_fluent( ushort_t f, pair_t **ptr ) const
{
  if( fluents( data_ ) == (pair_t*)&data_[size_] )
    {
      if( ptr ) *ptr = (pair_t*)&data_[size_];
      return( false );
    }
  else
    {
      register pair_t *first_ = fluents( data_ );
      register pair_t *last_ = (pair_t*)(&data_[size_]);
      register pair_t *middle_ = first_ + (last_ - first_) / 2;

      while( (first_ < last_) && (middle_->first != f) )
	if( first_ == middle_ )
	  {
	    if( middle_->first < f ) ++middle_;
	    break;
	  }
	else
	  {
	    first_ = (middle_->first<f?middle_:first_);
	    last_ = (middle_->first<f?last_:middle_);
	    middle_ = first_ + (last_- first_) / 2;
	  }

      if( ptr ) *ptr = middle_;
      return( (middle_ < (pair_t*)&data_[size_]) && (middle_->first == f) );
    }
}

void
state_t::insert_fluent( ushort_t f, Rational v, pair_t *ptr )
{
  unsigned nsize_ = size_ + sizeof(pair_t);
  char *ndata_ = (char*)malloc( nsize_ );
  ((ushort_t*)ndata_)[0] = ((ushort_t*)data_)[0];
  ptr = (!ptr?(pair_t*)&data_[size_]:ptr);

  // copy data
  char *base_ = (char*)predicates( data_ );
  char *nbase_ = (char*)predicates( ndata_ );
  memcpy( nbase_, base_, (char*)ptr - base_ );
  *(pair_t*)&nbase_[(char*)ptr-base_] = std::make_pair( f, v );
  memcpy( &nbase_[(char*)ptr-base_+sizeof(pair_t)],
	  &base_[(char*)ptr-base_],
	  size_ - (sizeof(ushort_t) + (char*)ptr - base_) );

  // set fields
  free( data_ );
  size_ = nsize_;
  data_ = ndata_;
}

void
state_t::delete_fluent( unsigned count, pair_t *ptr )
{
  if( count > 0 )
    {
      unsigned nsize_ = size_ - count * sizeof(pair_t);
      char *ndata_ = (char*)malloc( nsize_ );
      ((ushort_t*)ndata_)[0] = ((ushort_t*)data_)[0];

      // copy predicates
      memcpy( &((ushort_t*)ndata_)[1],
	      &((ushort_t*)data_)[1],
	      ((ushort_t*)data_)[0]*sizeof(ushort_t) );

      // copy fluents and rest
      pair_t *base_ = fluents( data_ );
      pair_t *nbase_ = fluents( ndata_ );
      memcpy( nbase_, base_, (ptr - base_) * sizeof(pair_t) );
      memcpy( &nbase_[ptr - base_],
	      &base_[count + (ptr - base_)],
	      nsize_ - sizeof(ushort_t) - (ptr - base_) * sizeof(pair_t) );

      // set fields
      free( data_ );
      size_ = nsize_;
      data_ = ndata_;
    }
}

bool
state_t::check( unsigned size, const char *data ) const
{
  ushort_t sz = 0;
  ushort_t *lp = NULL;
  for( ushort_t *p = predicates( data ); p < (ushort_t*)fluents( data ); lp = p++, ++sz )
    if( lp && (*lp >= *p) ) return( false );

  if( sz != *(ushort_t*)data ) return( false );

  pair_t *lf = NULL;
  for( pair_t *f = fluents( data ); f < (pair_t*)&data[size]; lf = f++ )
    if( lf && (lf->first >= f->first) ) return( false );

  return( true );
}

unsigned
state_t::digest( void ) const
{
  unsigned *ptr, result;
  unsigned char digest[16];
  MD4_CTX context;

  // compute MD4 digests
  MD4Init( &context );
  MD4Update( &context, (unsigned char*)data_, size_ );
  MD4Final( digest, &context );

  // compact digest into unsigned (assumes sizeof(unsigned) = 4)
  ptr = (unsigned*)digest;
  result = (ptr[0] ^ ptr[1] ^ ptr[2] ^ ptr[3]);
  return( result );
}

bool
state_t::holds( const Application &app, Rational value ) const
{
  pair_t *ptr;
  ushort_t f = problem_t::fluent_hash_get( app );
  if( !find_fluent( f, &ptr ) )
    return( value == 0 );
  else
    return( ptr->second == value );
}

void
state_t::add( ushort_t atom )
{
  ushort_t *ptr;
  if( !find_predicate( atom, &ptr ) )
    insert_predicate( atom, ptr );
  assert( make_check() );
}

void
state_t::add( const Application &app, Rational value )
{
  pair_t *ptr;
  ushort_t f =  problem_t::fluent_hash_get( app );
  if( find_fluent( f, &ptr ) )
    {
      if( value == 0 )
	delete_fluent( 1, ptr );
      else
	ptr->second = value;
    }
  else if( value != 0 )
    insert_fluent( f, value, ptr );
  assert( make_check() );
}

void
state_t::clear( ushort_t atom )
{
  ushort_t *ptr;
  if( find_predicate( atom, &ptr ) )
    {
      assert( (data_ < (char*)ptr) && (&data_[size_] > (char*)ptr) && (*ptr == atom) );
      delete_predicate( 1, ptr );
    }
  assert( make_check() );
}

const state_t&
state_t::next( const Action *action ) const
{
  state_t* nstate = new state_t( *this );
  if( action != NULL )
    action->affect( *nstate );
  nstate->make_digest();
  return( *nstate );
}

void
state_t::print( std::ostream &os ) const
{
  os << digest_ << ":[";

  for( ushort_t *p = predicates( data_ ); p < (ushort_t*)fluents( data_ ); ++p )
    {
      os << " ";
      if( *p % 2 ) os << "~";
      os << *p;
    }

  for( pair_t *f = fluents( data_ ); f < (pair_t*)&data_[size_]; ++f )
    os << " " << f->first << "=" << f->second;

  os << " ]";
}

void
state_t::full_print( std::ostream &os, const problem_t *problem ) const
{
  os << "[" << size_ << ":" << digest_ << "]";

  for( ushort_t *p = predicates( data_ ); p < (ushort_t*)fluents( data_ ); ++p )
    {
      os << " ";
      const Atom *atom = problem_t::atom_inv_hash_get( *p );
      os << *p << (*p%2?":(not ":":");
      problem->print( os, *atom );
      if( *p % 2 ) os << ")";
    }

  for( pair_t *f = fluents( data_ ); f < (pair_t*)&data_[size_]; ++f )
    {
      os << " ";
      const Application *app = problem_t::fluent_inv_hash_get( f->first );
      os << f->first << ":";
      problem->print( os, *app );
      os << "=" << f->second;
    }
}

void
state_t::printXML( std::ostream &os, bool goal ) const
{
  os << "<state>";
  if( goal ) os << "<is-goal/>";

#if 0
  for( AtomSet::const_iterator ai = atoms_.begin(); ai != atoms_.end(); ++ai )
    (*ai)->printXML( os, predicates, functions, terms );

  for( ValueMap::const_iterator vi = values_.begin(); vi != values_.end(); ++vi )
    {
      const Application *ap = vi->first;
      os << "<fluent><function>";
      functions.print_function( os, ap->function() );
      os << "</function>";

      for( size_t i = 0; i<ap->arity(); ++i )
	{
	  Term t = ap->term( i );
	  terms.print_term( os, t );
	}
      os << "<value>" << vi->second << "</value>";
      os << "</fluent>";
    }
#endif

  os << "</state>";
}

void 
state_t::send( std::ostream& os ) const
{
}
