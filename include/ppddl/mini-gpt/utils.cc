#include "states.h"
#include "utils.h"

extern "C" {
#include "md4.h"
};


/*******************************************************************************
 *
 * atom list
 *
 ******************************************************************************/

atomList_t::atomList_t( const atomList_t &alist )
{
  notify( this, "atomList_t::atomList_t(atomList_t&)" );
  if( alist.size() > 0 )
    {
      size_ = alist.size();
      data_ = (ushort_t*)malloc( size_ * sizeof(ushort_t) );
      for( size_t i = 0; i < size_; ++i )
	data_[i] = alist.atom( i );
      data_ptr_ = &data_[size_];
    }
}

atomList_t::atomList_t( const ushort_t *array, size_t sz )
{
  notify( this, "atomList_t::atomList_t(ushort_t&,size_t)" );
  size_ = sz;
  data_ = (ushort_t*)malloc( size_ * sizeof(ushort_t) );
  for( size_t i = 0; i < size_; ++i )
    data_[i] = array[i];
  data_ptr_ = &data_[size_];
}

unsigned
atomList_t::hash_value( void ) const
{
  unsigned *ptr, result;
  unsigned char digest[16];
  MD4_CTX context;

  // compute MD4 digests
  MD4Init( &context );
  MD4Update( &context, (unsigned char*)data_, size() );
  MD4Final( digest, &context );

  // compact digest into unsigned (assumes sizeof(unsigned) = 4)
  ptr = (unsigned*)digest;
  result = (ptr[0] ^ ptr[1] ^ ptr[2] ^ ptr[3]);
  return( result );
}

bool
atomList_t::equal( const ushort_t *array, size_t sz ) const
{
  if( size() != sz ) return( false );
  for( size_t i = 0; i < sz; ++i )
    if( array[i] != atom( i ) )
      return( false );
  return( true );
}

void
atomList_t::intersect( const atomList_t &alist )
{
  for( int i = 0; i < (int)size(); ++i )
    if( !alist.find( atom( i ) ) )
      {
	remove( atom( i ) );
	--i;
      }
}

bool
atomList_t::empty_intersection( const atomList_t &alist ) const
{
  for( size_t i = 0; i < alist.size(); ++i )
    if( find( alist.atom( i ) ) ) return( false );
  return( true );
}

size_t
atomList_t::intersection_size( const atomList_t &alist ) const
{
  size_t isize = 0;
  for( size_t i = 0; i < alist.size(); ++i )
    if( find( alist.atom( i ) ) ) ++isize;
  return( isize );
}

void
atomList_t::insert( const atomList_t &alist )
{
  for( size_t i = 0; i < alist.size(); ++i )
    insert( alist.atom( i ) );
}

void
atomList_t::remove( const atomList_t &alist )
{
  for( size_t i = 0; i < alist.size(); ++i )
    remove( alist.atom( i ) );
}

bool
atomList_t::holds( const state_t &state, bool nprec ) const
{
  for( size_t i = 0; i < size(); ++i )
    {
      if( nprec )
	{
	  if( !state.holds( atom( i ) ) )
	    return( false );
	}
      else
	{
	  ushort_t atm = atom( i );
	  if( ((atm%2) && state.holds( atm-1 )) || 
	      (!(atm%2) && !state.holds( atm )) )
	    return( false );
	}
    }
  return( true );
}

bool
atomList_t::holds( const atomList_t &alist, bool nprec ) const
{
  for( size_t i = 0; i < size(); ++i )
    {
      if( nprec )
	{
	  if( !alist.find( atom( i ) ) )
	    return( false );
	}
      else
	{
	  ushort_t atm = atom( i );
	  if( ((atm%2) && alist.find( atm-1 )) || 
	      (!(atm%2) && !alist.find( atm )) )
	    return( false );
	}
    }
  return( true );
}

void
atomList_t::print( std::ostream &os ) const
{
  if( size() == 0 )
    os << "[ <empty> ]";
  else
    {
      os << "[";
      for( size_t i = 0; i < size(); ++i )
	{
	  os << atom( i );
	  if( i + 1 < size() ) os << " ";
	}
      os << "]";
    }
}

bool
atomList_t::operator==( const atomList_t &alist ) const
{
  for( size_t i = 0; i < size(); ++i )
    if( !alist.find( atom(i) ) )
      return( false );
  for( size_t i = 0; i < alist.size(); ++i )
    if( !find( alist.atom(i) ) )
      return( false );
  return( true );
}

atomList_t&
atomList_t::operator=( const atomList_t &alist )
{
  clear();
  for( size_t i = 0; i < alist.size(); ++i )
    insert( alist.atom( i ) );
  return( *this );
}


/*******************************************************************************
 *
 * atom list list
 *
 ******************************************************************************/

bool
atomListList_t::find( const atomList_t &alist ) const
{
  for( size_t i = 0; i < size(); ++i )
    if( atom_list( i ) == alist ) return( true );
  return( false );
}

void
atomListList_t::insert( atomList_t *alist )
{
  size_t i;
  for( i = 0; i < size(); ++i )
    if( atom_list( i ) == *alist ) break;

  if( i == size() )
    {
      if( !data_ || (data_ptr_ == &data_[size_]) )
	{
	  size_ = (!data_ ? 1 : size_ << 1);
	  atomList_t **ndata_ =
	    (atomList_t**) realloc( data_, size_ * sizeof(atomList_t*) );
	  data_ptr_ = (!data_ ? ndata_ : &ndata_[data_ptr_ - data_]);
	  data_ = ndata_;
	}
      *data_ptr_++ = alist;
    }
}

bool
atomListList_t::holds( const state_t &state, bool nprec ) const
{
  for( size_t i = 0; i < size(); ++i )
    if( atom_list( i ).holds( state, nprec ) )
      return( true );
  return( false );
}

void
atomListList_t::print( std::ostream &os ) const
{
  os << "[";
  for( size_t i = 0; i < size(); ++i )
    {
      atom_list( i ).print( os );
      if( i + 1 < size() ) os << ",";
    }
  os << "]";
}

bool
atomListList_t::operator==( const atomListList_t &alist ) const
{
  for( size_t i = 0; i < size(); ++i )
    if( !alist.find( atom_list( i ) ) )
      return( false );
  for( size_t i = 0; i < alist.size(); ++i )
    if( !find( alist.atom_list( i ) ) )
      return( false );
  return( true );
}

atomListList_t&
atomListList_t::operator=( const atomListList_t &alist )
{
  clear();
  for( size_t i = 0; i < alist.size(); ++i )
    {
      atomList_t *al = new atomList_t;
      *al = alist.atom_list( i );
      insert( al );
    }
  return( *this );
}
