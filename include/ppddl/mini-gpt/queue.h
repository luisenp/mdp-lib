#ifndef QUEUE_H
#define QUEUE_H

#include <queue>
#include <set>


/*******************************************************************************
 *
 * priority queue
 *
 ******************************************************************************/

template<typename _type,typename _index>
class priorityQueue_t
{
public:
  class entry_t
  {
  protected:
    _type content_;
    entry_t *prev_, *next_;
  public:
    entry_t( const _type &element ) : prev_(0), next_(0)
      {
	content_ = element;
      }
    const _type contents( void ) const { return( content_ ); }
    friend class priorityQueue_t;
  };

  typedef typename priorityQueue_t<_type,_index>::entry_t entry_reference;
  typedef const typename priorityQueue_t<_type,_index>::entry_t const_entry_reference;

private:
  size_t size_;
  size_t table_size_;
  entry_reference **table_;
  std::priority_queue<unsigned,std::vector<unsigned>,std::greater<unsigned> > pq_;
  _index index_;

  void resize( size_t index )
    {
      size_t nsize = table_size_;
      for( ; nsize <= index; nsize = nsize << 1 );
      table_ = (entry_reference**)realloc( table_, nsize * sizeof(entry_reference*) );
      memset( &table_[table_size_], 0, (nsize - table_size_) * sizeof(entry_reference*) );
      table_size_ = nsize;
    }

public:
  priorityQueue_t( size_t table_size = 128 )
    {
      size_ = 0;
      table_size_ = table_size;
      table_ = (entry_reference**)calloc( table_size_, sizeof(entry_reference*) );
    }
  ~priorityQueue_t()
    {
      while( !empty() ) pop();
      free( table_ );
    }

  const_entry_reference* find( const _type &element, unsigned &bucket ) const
    {
      bucket = index_( element );
      if( bucket >= table_size_ ) return( NULL );
      for( const_entry_reference *ptr = table_[bucket]; ptr != NULL; ptr = ptr->next_ )
	if( ptr->contents() == element ) return( ptr );
      return( NULL );
    }
  void remove( const _type &element )
    {
      unsigned bucket;
      const_entry_reference *entry = find( element, bucket );
      if( entry != NULL )
	{
	  if( entry->prev_ == NULL )
	    table_[bucket] = entry->next_;
	  else
	    entry->prev_->next_ = entry->next_;
	  
	  if( entry->next_ != NULL )
	    entry->next_->prev_ = entry->prev_;

	  if( table_[bucket] == NULL )
	    {
	      // remove bucket from pq_
	      std::set<unsigned> iset;
	      for( ; !pq_.empty(); pq_.pop() )
		if( pq_.top() != bucket )
		  iset.insert( pq_.top() );

	      std::set<unsigned>::const_iterator it;
	      for( it = iset.begin() ; it != iset.end(); ++it )
		pq_.push( *it );
	    }

	  --size_;
	  delete entry;
	}
    }
  bool empty( void ) const { return( pq_.empty() ); }
  size_t size( void ) const { return( size_ ); }
  const _type top( void ) const
    {
      return( table_[pq_.top()]->contents() );
    }
  void pop( void ) { remove( top() ); }
  void push( const _type &element )
    {
      unsigned bucket = index_( element );
      entry_reference *entry = new entry_reference( element );
      if( bucket >= table_size_ ) resize( bucket );
      entry->next_ = table_[bucket];
      if( table_[bucket] != NULL )
	table_[bucket]->prev_ = entry;
      else
	pq_.push( bucket );
      table_[bucket] = entry;
      ++size_;
    }
  void print_indices( std::ostream &os ) const
    {
      std::set<unsigned> iset;
      for( size_t i = 0; i < table_size_; ++i )
	if( table_[i] != NULL ) iset.insert( i );
      for( std::set<unsigned>::const_iterator it = iset.begin(); it != iset.end(); )
	{
	  os << *it;
	  if( ++it != iset.end() )
	    os << " ";
	}
    }
};

#endif // QUEUE_H
