#ifndef GRAPH_H
#define GRAPH_H

#include "global.h"
#include "utils.h"
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <iostream>
#include <vector>


/*******************************************************************************
 *
 * undirected graph
 *
 ******************************************************************************/

class graph_t
{
private:
  ushort_t nodes_;
  unsigned size_;
  uchar_t *edges_;

  void maximal_cliques_aux( ushort_t i, atomList_t &clique,
			    std::vector<atomList_t*> &cliques,
                            size_t max_size, bool (*constraint)( atomList_t& ) ) const;
  void maximal_disjoint_cliques_aux( ushort_t i, atomList_t &clique,
				     atomList_t &vertices,
				     std::vector<atomList_t*> &cliques,
				     size_t max_size, bool (*constraint)( atomList_t& ) ) const;
  void connected_components_aux( ushort_t i, atomList_t &component ) const;
  void subsets_aux( unsigned i, size_t m, atomList_t &subset,
		    std::vector<atomList_t*> &subsets ) const;

public:
  graph_t( ushort_t nodes ) : nodes_(nodes)
    {
      unsigned entries = nodes_ * (nodes_ - 1) / 2;
      size_ = 1 + (entries / (8*sizeof(uchar_t)));
      edges_ = (uchar_t*)calloc( size_, sizeof(uchar_t) );
    }
  graph_t( const graph_t &graph );
  ~graph_t()
    {
      free( edges_ );
    }

  unsigned idx( ushort_t i, ushort_t j ) const
    {
      ushort_t m = MIN(i,j), M = MAX(i,j);
      unsigned index = m*(nodes_-2) + M - (m*(m-1))/2 - 1;
      return( index / (8*sizeof(uchar_t)) );
    }
  unsigned off( ushort_t i, ushort_t j ) const
    {
      ushort_t m = MIN(i,j), M = MAX(i,j);
      unsigned index = m*(nodes_-2) + M - (m*(m-1))/2 - 1;
      return( index % (8*sizeof(uchar_t)) );
    }
  unsigned decode( unsigned index ) const
    {
      unsigned k, m = 0;
      for( k = nodes_ - 1; index >= m+k; m += k, --k );
      ushort_t i = nodes_ - k - 1;
      ushort_t j = index + 1 - (i*(2*nodes_ - 3 - i))/2;
      return( (i<<16) + j );
    }

  void add_edge( ushort_t i, ushort_t j )
    {
      edges_[idx(i,j)] = edges_[idx(i,j)] | (1<<off(i,j));
    }
  bool edge( ushort_t i, ushort_t j ) const
    {
      return( edges_[idx(i,j)] & (1<<off(i,j)) );
    }
  void closure( void );
  void complement( void );
  void maximal_cliques( std::vector<atomList_t*> &cliques,
                        size_t max_size = UINT_MAX,
			bool (*constraint)( atomList_t& ) = NULL ) const
    {
      atomList_t clique;
      try
        {
          maximal_cliques_aux( 0, clique, cliques, max_size, constraint );
        }
      catch( int e )
        {
        }
    }
  void maximal_disjoint_cliques( std::vector<atomList_t*> &cliques,
				 size_t max_size = UINT_MAX,
				 bool (*constraint)( atomList_t& ) = NULL ) const
    {
      atomList_t vertices, clique;
      try
        {
          maximal_disjoint_cliques_aux( 0, clique, vertices, cliques, max_size, constraint );
        }
      catch( int e )
        {
        }
    }
  void connected_components( std::vector<atomList_t*> &components ) const
    {
      atomList_t visited, component;
      for( ushort_t i = 0; i < nodes_; ++i )
	if( !visited.find( i ) )
	  {
	    connected_components_aux( i, component );
	    visited.insert( component );
	    atomList_t *comp = new atomList_t( component );
	    components.push_back( comp );
	    component.clear();
	  }
    }
  void subsets( size_t m, std::vector<atomList_t*> &subsets ) const
    {
      atomList_t subset;
      subsets_aux( 0, m, subset, subsets );
    }
  void print( std::ostream &os ) const;
};


/*******************************************************************************
 *
 * directed graph
 *
 ******************************************************************************/

class digraph_t
{
private:
  ushort_t nodes_;
  unsigned size_;
  uchar_t *edges_;

public:
  digraph_t( ushort_t nodes ) : nodes_(nodes)
    {
      unsigned entries = nodes_ * nodes_;
      size_ = 1 + (entries / (8*sizeof(uchar_t)));
      edges_ = (uchar_t*)calloc( size_, sizeof(uchar_t) );
    }
  digraph_t( const digraph_t &graph );
  ~digraph_t()
    {
      free( edges_ );
    }

  unsigned idx( ushort_t i, ushort_t j ) const
    {
      return( (i*nodes_ + j) / (8*sizeof(uchar_t)) );
    }
  unsigned off( ushort_t i, ushort_t j ) const
    {
      return( (i*nodes_ + j) % (8*sizeof(uchar_t)) );
    }
  unsigned decode( unsigned index ) const
    {
      ushort_t i = index / nodes_;
      ushort_t j = index % nodes_;
      return( (i<<16) + j );
    }

  void add_edge( ushort_t i, ushort_t j )
    {
      edges_[idx(i,j)] = edges_[idx(i,j)] | (1<<off(i,j));
    }
  void remove_edge( ushort_t i, ushort_t j )
    {
      edges_[idx(i,j)] = edges_[idx(i,j)] & ~(1<<off(i,j));
    }
  bool edge( ushort_t i, ushort_t j ) const
    {
      return( edges_[idx(i,j)] & (1<<off(i,j)) );
    }
  bool source( ushort_t i ) const
    {
      for( ushort_t j = 0; j < nodes_; ++j )
	if( (i != j) && edge(j,i) ) return( false );
      return( true );
    }
  bool sink( ushort_t i ) const
    {
      for( ushort_t j = 0; j < nodes_; ++j )
	if( (i != j) && edge(i,j) ) return( false );
      return( true );
    }
  bool empty( void ) const
    {
      for( size_t i = 0; i < size_; ++i )
	if( edges_[i] ) return( false );
      return( true );
    }
  void closure( void );
  bool acyclic( void ) const;
  bool remove_edges_at( ushort_t i )
    {
      bool rv = false;
      for( ushort_t j = 0; j < nodes_; ++j )
	if( i != j )
	  {
	    rv = rv || edge(i,j) || edge(j,i);
	    remove_edge( i, j );
	    remove_edge( j, i );
	  }
      return( rv );
    }
  void layer( std::vector<atomList_t*> &layers ) const;
  void print( std::ostream &os ) const;
};

#endif // GRAPH_H
