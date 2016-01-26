#include "graph.h"


/*******************************************************************************
 *
 * undirected graph
 *
 ******************************************************************************/

graph_t::graph_t( const graph_t &graph )
{
  unsigned entries = graph.nodes_ * (graph.nodes_ - 1) / 2;
  size_ = 1 + (entries / (8*sizeof(uchar_t)));
  edges_ = (uchar_t*)calloc( size_, sizeof(uchar_t) );

  for( ushort_t i = 0; i < nodes_; ++i )
    for( ushort_t j = 0; j < nodes_; ++j )
      if( (i != j) && graph.edge(i,j) )
	add_edge( i, j );
}

void
graph_t::closure( void )
{
  bool change = true;
  while( change )
    {
      change = false;
      for( ushort_t i = 0; i < nodes_; ++i )
	for( ushort_t j = 0; j < nodes_; ++j ) // i+1
	  if( (i != j) && edge(i,j) )
	    for( ushort_t k = 0; k < nodes_; ++k ) // i+1
	      if( (i != k) && (j != k) && edge(j,k) && !edge(i,k) )
		{
		  change = true; 
		  add_edge( i, k );
		}
    }
}

void
graph_t::complement( void )
{
  for( unsigned i = 0; i < size_; ++i )
    edges_[i] = ~edges_[i];
}

void
graph_t::maximal_cliques_aux( ushort_t i, atomList_t &clique,
			      std::vector<atomList_t*> &cliques,
			      size_t max_size, bool (*constraint)( atomList_t& ) ) const
{
  if( (i == nodes_) && (!constraint || constraint( clique )) )
    {
      size_t j;
      for( j = 0; j < cliques.size(); ++j )
	if( clique.intersection_size( *cliques[j] ) == clique.size() )
	  break;

      if( j == cliques.size() )
	{
	  atomList_t *K = new atomList_t( clique );
	  cliques.push_back( K );
	  std::cout << "new clique" << std::endl;
          if( cliques.size() == max_size ) throw 0;
	}
    }
  else if( i < nodes_ )
    {
      size_t j;
      for( j = 0; j < clique.size(); ++j )
	if( !edge( i, clique.atom(j) ) ) break;

      if( j == clique.size() )
	{
	  clique.insert( i );
	  maximal_cliques_aux( 1+i, clique, cliques, max_size, constraint );
	  clique.remove( i );
	  maximal_cliques_aux( 1+i, clique, cliques, max_size, constraint );
	}
      else
	maximal_cliques_aux( 1+i, clique, cliques, max_size, constraint );
    }
}

void
graph_t::maximal_disjoint_cliques_aux( ushort_t i, atomList_t &clique,
				       atomList_t &vertices,
				       std::vector<atomList_t*> &cliques,
				       size_t max_size, bool (*constraint)( atomList_t& ) ) const
{
  if( (i == nodes_) && (!constraint || constraint( clique )) )
    {
      size_t j;
      for( j = 0; j < cliques.size(); ++j )
	if( clique.intersection_size( *cliques[j] ) == clique.size() )
	  break;

      if( j == cliques.size() )
	{
	  atomList_t *K = new atomList_t( clique );
	  cliques.push_back( K );
	  vertices.insert( clique );
          if( cliques.size() == max_size ) throw 0;
	}
    }
  else if( i < nodes_ )
    {
      size_t j;
      for( j = 0; j < clique.size(); ++j )
	if( !edge( i, clique.atom(j) ) ) break;

      if( (j == clique.size()) && !vertices.find( i ) )
	{
	  clique.insert( i );
	  maximal_disjoint_cliques_aux( 1+i, clique, vertices, cliques, max_size, constraint );
	  clique.remove( i );
	  maximal_disjoint_cliques_aux( 1+i, clique, vertices, cliques, max_size, constraint );
	}
      else
	maximal_disjoint_cliques_aux( 1+i, clique, vertices, cliques, max_size, constraint );
    }
}

void
graph_t::connected_components_aux( ushort_t i, atomList_t &component ) const
{
  if( !component.find( i ) )
    {
      component.insert( i );
      for( ushort_t j = 0; j < nodes_; ++j )
	if( (i != j) && edge(i,j) )
	  connected_components_aux( j, component );
    }
}

void
graph_t::subsets_aux( unsigned i, size_t m, atomList_t &subset,
		      std::vector<atomList_t*> &subsets ) const
{
  if( (i == nodes_) || (m == subset.size()) )
    {
      if( (m == subset.size()) || (nodes_ == subset.size()) )
	{
	  atomList_t *sset = new atomList_t( subset );
	  subsets.push_back( sset );
	}
    }
  else
    {
      subset.insert( i );
      subsets_aux( 1+i, m, subset, subsets );
      subset.remove( i );
      subsets_aux( 1+i, m, subset, subsets );
    }
}

void
graph_t::print( std::ostream &os ) const
{
  os << "nodes = {";
  for( ushort_t i = 0; i < nodes_; ++i )
    os << " " << i;
  os << " }" << std::endl << "edges = {";
  for( unsigned i = 0; i < size_; ++i )
    {
      ushort_t k = 0;
      for( ushort_t j = edges_[i]; j > 0; j = j>>1, ++k )
	if( j % 2 )
	  {
	    unsigned p = decode( i*8*sizeof(uchar_t) + k );
	    ushort_t x = p >> 16;
	    ushort_t y = p & 0xFFFF;
	    if( (x < nodes_) && (y < nodes_) )
	      os << " (" << x << "," << y << ")";
	  }
    }
  os << " }" << std::endl;
}


/*******************************************************************************
 *
 * directed graph
 *
 ******************************************************************************/

digraph_t::digraph_t( const digraph_t &graph )
{
  nodes_ = graph.nodes_;
  unsigned entries = nodes_ * nodes_;
  size_ = 1 + (entries / (8*sizeof(uchar_t)));
  edges_ = (uchar_t*)calloc( size_, sizeof(uchar_t) );

  for( ushort_t i = 0; i < nodes_; ++i )
    for( ushort_t j = 0; j < nodes_; ++j )
      if( (i != j) && graph.edge(i,j) )
	add_edge( i, j );
}

void
digraph_t::closure( void )
{
  bool change = true;
  while( change )
    {
      change = false;
      for( ushort_t i = 0; i < nodes_; ++i )
	for( ushort_t j = 0; j < nodes_; ++j )
	  if( (i != j) && edge(i,j) )
	    for( ushort_t k = 0; k < nodes_; ++k )
	      if( (i != k) && (j != k) && edge(j,k) && !edge(i,k) )
		{
		  change = true; 
		  add_edge( i, k );
		}
    }
}

bool
digraph_t::acyclic( void ) const
{
  // copy graph
  digraph_t graph( nodes_ );
  for( ushort_t i = 0; i < nodes_; ++i )
    for( ushort_t j = 0; j < nodes_; ++j )
      if( (i != j) && edge(i,j) )
	graph.add_edge( i, j );

  // iteratively remove sources
  bool change = true;
  while( change )
    {
      change = false;
      for( ushort_t i = 0; i < nodes_; ++i )
	if( graph.source( i ) )
	  {
	    change = graph.remove_edges_at( i ) || change;
	  }
    }
  return( graph.empty() );
}

void
digraph_t::layer( std::vector<atomList_t*> &layers ) const
{
  // copy graph
  digraph_t graph( nodes_ );
  for( ushort_t i = 0; i < nodes_; ++i )
    for( ushort_t j = 0; j < nodes_; ++j )
      if( (i != j) && edge(i,j) )
	graph.add_edge( i, j );

  // extract layers
  atomList_t processed;
  while( !graph.empty() )
    {
      atomList_t *layer = new atomList_t;
      for( ushort_t i = 0; i < nodes_; ++i )
	if( !processed.find( i ) && graph.sink( i ) )
	  {
	    processed.insert( i );
	    layer->insert( i );
	  }
      assert( layer->size() > 0 );
      layers.push_back( layer );

      for( ushort_t i = 0; i < layer->size(); ++i )
	graph.remove_edges_at( layer->atom( i ) );
    }

  // extract final layer
  atomList_t *layer = new atomList_t;
  for( ushort_t i = 0; i < nodes_; ++i )
    if( !processed.find( i ) )
      layer->insert( i );
  assert( layer->size() > 0 );
  layers.push_back( layer );
}

void
digraph_t::print( std::ostream &os ) const
{
  os << "nodes = {";
  for( ushort_t i = 0; i < nodes_; ++i )
    os << " " << i;
  os << " }" << std::endl << "edges = {";
  for( unsigned i = 0; i < size_; ++i )
    {
      ushort_t k = 0;
      for( ushort_t j = edges_[i]; j > 0; j = j>>1, ++k )
	if( j % 2 )
	  {
	    unsigned p = decode( i*8*sizeof(uchar_t) + k );
	    ushort_t x = p >> 16;
	    ushort_t y = p & 0xFFFF;
	    if( (x < nodes_) && (y < nodes_) )
	      os << " (" << x << "," << y << ")";
	  }
    }
  os << " }" << std::endl;
}
