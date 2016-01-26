#ifndef HASH_H
#define HASH_H

#include "global.h"
#include "heuristics.h"
#include "states.h"

extern "C" {
#include "md4.h"
};

class action_t;
class state_t;
class problem_t;
class hash_t;


/*******************************************************************************
 *
 * hash entry
 *
 ******************************************************************************/

class hashEntry_t
{
  double value_;
  unsigned bits_;
  const state_t *state_;
  hashEntry_t *next_, *prev_;
  friend class hash_t;

public:
  hashEntry_t() : value_(0), bits_(0), state_(NULL), next_(0), prev_(0) { }
  hashEntry_t( const state_t &state, double value = 0 )
    : value_(value), bits_(0), next_(0), prev_(0)
    {
      notify( this, "hashEntry_t::hashEntry_t(state_t&,double)" );
      state_ = state_t::get_state( state );
    }
  virtual ~hashEntry_t() { }

  void update( double value )
    {
      //double nvalue = MAX(value_,value);
      value_ = MIN(value,(double)gpt::dead_end_value);
    }
  double value( void ) const { return( value_ ); }
  unsigned bits( void ) const { return( bits_ ); }
  void set_bits( unsigned bits ) { bits_ = bits; }
  const state_t* state( void ) const { return( state_ ); }
  const hashEntry_t* next( void ) const { return( next_ ); }
};


/*******************************************************************************
 *
 * hash entry (extended)
 *
 ******************************************************************************/

class hashEntryX_t : public hashEntry_t
{
  ushort_t data_[2];
  hashEntryX_t *elink_;

public:
  hashEntryX_t() : hashEntry_t() { data_[0] = USHORT_MAX; data_[1] = USHORT_MAX; }
  hashEntryX_t( const state_t &state, double value = 0 ) : hashEntry_t(state,value)
    {
      data_[0] = USHORT_MAX;
      data_[1] = USHORT_MAX;
    }
  virtual ~hashEntryX_t() { }

  unsigned low( void ) const { return( data_[0] ); }
  void set_low( unsigned low ) { data_[0] = low; }
  unsigned idx( void ) const { return( data_[1] ); }
  void set_idx( unsigned idx ) { data_[1] = idx; }
  hashEntryX_t* elink( void ) { return( elink_ ); }
  void set_elink( hashEntryX_t *elink ) { elink_ = elink; }
};


/*******************************************************************************
 *
 * hash
 *
 ******************************************************************************/

class hash_t
{
  bool ext_;
  unsigned size_;
  unsigned dimension_;
  unsigned *number_;
  mutable heuristic_t *heuristic_;
  hashEntry_t **table_;
  std::pair<state_t*,Rational> **display_;

  void rehash( void );

public:
  hash_t()
    : ext_(false), size_(0), dimension_(0), number_(0), heuristic_(0), table_(0) { }
  hash_t( unsigned dimension, heuristic_t &heuristic );
  virtual ~hash_t();

  void set_extended( void ) { ext_ = true; }
  unsigned hash_value( const state_t &state ) const { return(state.hash_value()%dimension_); }
  double heuristic( const state_t &state ) const { return( heuristic_->value( state ) ); }
  hashEntry_t* find( const state_t &state ) const
    {
      unsigned idx = hash_value( state );
      for( hashEntry_t *ptr = table_[idx]; ptr != NULL; ptr = ptr->next_ )
	if( *(ptr->state_) == state ) return( ptr );
      return( NULL );
    }
  hashEntry_t* insert( const state_t &state )
    {
      hashEntry_t *entry;
      //xxxxxx if( 4*size_ > 3*dimension_ ) rehash();
      if( !ext_ )
	entry = new hashEntry_t( state, heuristic( state ) );
      else 
	entry = new hashEntryX_t( state, heuristic( state ) );
      insert( entry );
      return( entry );
    }
  void insert( hashEntry_t *entry )
    {
      unsigned idx = hash_value( *entry->state() );
      entry->next_ = table_[idx];
      entry->prev_ = NULL;
      if( table_[idx] )
	table_[idx]->prev_ = entry;
      table_[idx] = entry;
      ++number_[idx];
      ++size_;
    }
  hashEntry_t* get( const state_t &state )
    {
      hashEntry_t *result = find( state );
      return( !result ? insert( state ) : result );
    }
  double value( const state_t &state )
    {
      const hashEntry_t *entry = (gpt::hash_all?get( state ):find( state ));
      if( !entry )
	return( heuristic( state ) );
      else
	return( entry->value() );
    }

  unsigned dimension( void ) const { return( dimension_ ); }
  unsigned size( void ) const { return( size_ ); }
  unsigned diameter( void ) const;
  void print( std::ostream &os, const problem_t &problem ) const;
  void dump( std::ostream &os ) const;

  virtual int bestAction( const state_t &state,
			  const problem_t &problem,
			  double &val,
			  const atomList_t *appliedActions = NULL );
  virtual double bestQValue( const state_t &state,
			     const problem_t &problem,
			     const atomList_t *appliedActions = NULL )
    {
      double value;
      bestAction( state, problem, value, appliedActions );
      return( value );
    }

public: // iterator
  class const_iterator;
  friend class hash_t::const_iterator;

  class const_iterator
  {
    const hash_t *hash_;
    const hashEntry_t *ptr_;
    size_t idx_;

  protected:
    const_iterator( const hash_t *h, int pos ) : hash_(h), ptr_(0), idx_(0)
      {
	if( pos == 0 )
	  {
	    for( ; idx_ < hash_->dimension_; ++idx_ )
	      if( (ptr_ = hash_->table_[idx_]) )
		break;
	  }
      }

  public:
    const_iterator() : hash_(0), ptr_(0), idx_(0) { }
    const hashEntry_t* operator*( void ) const { return( ptr_ ); }
    const_iterator operator++( void ) // pre increment
      {
	if( ptr_ != NULL )
	  {
	    if( !(ptr_ = ptr_->next()) )
	      {
		for( ++idx_; idx_ < hash_->dimension_; ++idx_ )
		  if( (ptr_ = hash_->table_[idx_]) )
		    break;
	      }
	  }
	return( *this );
      }
    const_iterator operator++( int ) // post increment
      {
	const_iterator it( *this ); // use default copy const.
	++(*this);
	return( it );
      }
    bool operator==( const const_iterator &it ) const
      {
	return( (ptr_ == it.ptr_) && (idx_ == it.idx_) );
      }
    bool operator!=( const const_iterator &it ) const
      {
	return( (ptr_ != it.ptr_) || (idx_ != it.idx_) );
      }
    friend class hash_t;
  };
  const const_iterator begin( void ) const
    {
      return( const_iterator( this, 0 ) );
    }
  const const_iterator end( void ) const
    {
      return( const_iterator( this, 1 ) );
    }
};


/*******************************************************************************
 *
 * state hash entry
 *
 ******************************************************************************/

class stateHashEntry_t
{
  const state_t *state_;
  stateHashEntry_t *next_, *prev_;
  friend class stateHash_t;

public:
  stateHashEntry_t() : state_(NULL), next_(0), prev_(0) { }
  stateHashEntry_t( const state_t &state ) : next_(0), prev_(0)
    {
      notify( this, "stateHashEntry_t::stateHashEntry_t(state_t&)" );
      state_ = new state_t( state );
    }
  ~stateHashEntry_t() { delete state_; }
  const state_t* state( void ) const { return( state_ ); }
  const stateHashEntry_t* next( void ) const { return( next_ ); }
};


/*******************************************************************************
 *
 * state hash
 *
 ******************************************************************************/

class stateHash_t
{
protected:
  unsigned size_;
  unsigned dimension_;
  unsigned *number_;
  stateHashEntry_t **table_;

  void rehash( void );

public:
  stateHash_t() : size_(0), dimension_(0), number_(0), table_(0) { }
  stateHash_t( unsigned dimension );
  ~stateHash_t();

  unsigned hash_value( const state_t &state ) const { return(state.hash_value()%dimension_); }
  stateHashEntry_t* find( const state_t &state ) const
    {
      unsigned idx = hash_value( state );
      for( stateHashEntry_t *ptr = table_[idx]; ptr != NULL; ptr = ptr->next_ )
	if( *(ptr->state_) == state ) return( ptr );
      return( NULL );
    }
  stateHashEntry_t* insert( const state_t &state )
    {
      //xxxxxx if( 4*size_ > 3*dimension_ ) rehash();
      stateHashEntry_t *entry = new stateHashEntry_t( state );
      insert( entry );
      return( entry );
    }
  void insert( stateHashEntry_t *entry )
    {
      unsigned idx = hash_value( *entry->state() );
      entry->next_ = table_[idx];
      entry->prev_ = NULL;
      if( table_[idx] )
	table_[idx]->prev_ = entry;
      table_[idx] = entry;
      ++number_[idx];
      ++size_;
    }
  stateHashEntry_t* get( const state_t &state )
    {
      stateHashEntry_t *result = find( state );
      return( !result ? insert( state ) : result );
    }

  unsigned dimension( void ) const { return( dimension_ ); }
  unsigned size( void ) const { return( size_ ); }
  unsigned diameter( void ) const;
  void print( std::ostream &os, const problem_t &problem ) const;
  void dump( std::ostream &os ) const;

public: // iterator
  class const_iterator;
  friend class stateHash_t::const_iterator;

  class const_iterator
  {
    const stateHash_t *hash_;
    const stateHashEntry_t *ptr_;
    size_t idx_;

  protected:
    const_iterator( const stateHash_t *shash, int pos ) : hash_(shash), ptr_(0), idx_(0)
      {
	if( pos == 0 )
	  {
	    for( ; idx_ < hash_->dimension_; ++idx_ )
	      if( (ptr_ = hash_->table_[idx_]) )
		break;
	  }
	else
	  {
	    idx_ = hash_->dimension_;
	    ptr_ = 0;
	  }
      }

  public:
    const_iterator() : hash_(0), ptr_(0), idx_(0) { }
    const state_t* operator*( void ) const { return( ptr_->state() ); }
    const_iterator operator++( void ) // pre increment
      {
	if( ptr_ != NULL )
	  {
	    if( !(ptr_ = ptr_->next()) )
	      {
		for( ++idx_; idx_ < hash_->dimension_; ++idx_ )
		  if( (ptr_ = hash_->table_[idx_]) )
		    break;
	      }
	  }
	return( *this );
      }
    const_iterator operator++( int ) // post increment
      {
	const_iterator it( *this ); // use default copy const.
	++(*this);
	return( it );
      }
    bool operator==( const const_iterator &it ) const
      {
	return( (ptr_ == it.ptr_) && (idx_ == it.idx_) );
      }
    bool operator!=( const const_iterator &it ) const
      {
	return( (ptr_ != it.ptr_) || (idx_ != it.idx_) );
      }
    friend class stateHash_t;
  };
  const const_iterator begin( void ) const
    {
      return( const_iterator( this, 0 ) );
    }
  const const_iterator end( void ) const
    {
      return( const_iterator( this, 1 ) );
    }
};


/*******************************************************************************
 *
 * atomlist hash entry
 *
 ******************************************************************************/

class atomListHashEntry_t
{
  double value_;
  unsigned bits_;
  const atomList_t *alist_;
  atomListHashEntry_t *next_, *prev_;
  friend class atomListHash_t;

public:
  atomListHashEntry_t() : value_(0), bits_(0), alist_(0), next_(0), prev_(0) { }
  atomListHashEntry_t( const ushort_t *array, size_t size )
    : value_(0), bits_(0), next_(0), prev_(0)
    {
      notify( this, "atomListHashEntry_t::atomListHashEntry_t(ushort_t*,size_t)" );
      alist_ = new atomList_t( array, size );
    }
  ~atomListHashEntry_t() { delete alist_; }
  void update( double value ) { value_ = MIN(value,(double)gpt::dead_end_value); }
  double value( void ) const { return( value_ ); }
  unsigned bits( void ) const { return( bits_ ); }
  void set_bits( unsigned bits ) { bits_ = bits; }
  const atomList_t* alist( void ) const { return( alist_ ); }
  const atomListHashEntry_t* next( void ) const { return( next_ ); }
};


/*******************************************************************************
 *
 * atomlist hash
 *
 ******************************************************************************/

class atomListHash_t
{
protected:
  unsigned size_;
  unsigned dimension_;
  unsigned *number_;
  atomListHashEntry_t **table_;

  void rehash( void );

public:
  atomListHash_t() : size_(0), dimension_(0), number_(0), table_(0) { }
  atomListHash_t( unsigned dimension );
  ~atomListHash_t();

  unsigned hash_value( const atomList_t &alist ) const
    {
      return( alist.hash_value() % dimension_ );
    }
  unsigned hash_value( const ushort_t *array, size_t size ) const
    {
      unsigned *ptr, result;
      unsigned char digest[16];
      MD4_CTX context;

      // compute MD4 digests
      MD4Init( &context );
      MD4Update( &context, (unsigned char*)array, size );
      MD4Final( digest, &context );

      // compact digest into unsigned (assumes sizeof(unsigned) = 4)
      ptr = (unsigned*)digest;
      result = (ptr[0] ^ ptr[1] ^ ptr[2] ^ ptr[3]);
      return( result % dimension_ );
    }
  atomListHashEntry_t* find( const ushort_t *array, size_t size ) const
    {
      unsigned idx = hash_value( array, size );
      for( atomListHashEntry_t *ptr = table_[idx]; ptr != NULL; ptr = ptr->next_ )
	if( ptr->alist_->equal( array, size ) ) return( ptr );
      return( NULL );
    }
  atomListHashEntry_t* insert( const ushort_t *array, size_t size )
    {
      //xxxxxx if( 4*size_ > 3*dimension_ ) rehash();
      atomListHashEntry_t *entry = new atomListHashEntry_t( array, size );
      insert( entry );
      return( entry );
    }
  void insert( atomListHashEntry_t *entry )
    {
      unsigned idx = hash_value( *entry->alist() );
      entry->next_ = table_[idx];
      entry->prev_ = NULL;
      if( table_[idx] )
	table_[idx]->prev_ = entry;
      table_[idx] = entry;
      ++number_[idx];
      ++size_;
    }
  atomListHashEntry_t* get( const ushort_t *array, size_t size )
    {
      atomListHashEntry_t *result = find( array, size );
      return( !result ? insert( array, size ) : result );
    }

  unsigned dimension( void ) const { return( dimension_ ); }
  unsigned size( void ) const { return( size_ ); }
  unsigned diameter( void ) const;
  void print( std::ostream &os, const problem_t &problem ) const;
  void dump( std::ostream &os ) const;

public: // iterator
  class const_iterator;
  friend class atomListHash_t::const_iterator;

  class const_iterator
  {
    const atomListHash_t *hash_;
    const atomListHashEntry_t *ptr_;
    size_t idx_;

  protected:
    const_iterator( const atomListHash_t *ahash, int pos ) : hash_(ahash), ptr_(0), idx_(0)
      {
	if( pos == 0 )
	  {
	    for( ; idx_ < hash_->dimension_; ++idx_ )
	      if( (ptr_ = hash_->table_[idx_]) )
		break;
	  }
      }

  public:
    const_iterator() : hash_(0), ptr_(0), idx_(0) { }
    const atomList_t* operator*( void ) const { return( ptr_->alist() ); }
    const_iterator operator++( void ) // pre increment
      {
	if( ptr_ != NULL )
	  {
	    if( !(ptr_ = ptr_->next()) )
	      {
		for( ++idx_; idx_ < hash_->dimension_; ++idx_ )
		  if( (ptr_ = hash_->table_[idx_]) )
		    break;
	      }
	  }
	return( *this );
      }
    const_iterator operator++( int ) // post increment
      {
	const_iterator it( *this ); // use default copy const.
	++(*this);
	return( it );
      }
    bool operator==( const const_iterator &it ) const
      {
	return( (ptr_ == it.ptr_) && (idx_ == it.idx_) );
      }
    bool operator!=( const const_iterator &it ) const
      {
	return( (ptr_ != it.ptr_) || (idx_ != it.idx_) );
      }
    friend class atomListHash_t;
  };
  const const_iterator begin( void ) const
    {
      return( const_iterator( this, 0 ) );
    }
  const const_iterator end( void ) const
    {
      return( const_iterator( this, 1 ) );
    }
};

#endif // HASH_H
