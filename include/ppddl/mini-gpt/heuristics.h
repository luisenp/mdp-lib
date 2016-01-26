#ifndef HEURISTICS_H
#define HEURISTICS_H

#include "actions.h"
#include "global.h"
#include "rational.h"
#include <stdlib.h>
#include <iostream>
#include <queue>
#include <set>
#include <tr1/unordered_map>

class atomList_t;
class atomListHash_t;
class atomListHashEntry_t;
class deterministicAction_t;
class actionList_t;
class problem_t;
class state_t;
class hash_t;
class algorithm_t;


/*******************************************************************************
 *
 * heuristic (abstract class)
 *
 ******************************************************************************/

class heuristic_t
{
protected:
  const problem_t &problem_;

public:
  heuristic_t( const problem_t &problem ) : problem_(problem) { }
  virtual ~heuristic_t() { }
  virtual double value( const state_t &state ) = 0;
  virtual void statistics( std::ostream &os ) const = 0;
};


/*******************************************************************************
 *
 * zero heuristic
 *
 ******************************************************************************/

class zeroHeuristic_t : public heuristic_t
{
public:
  zeroHeuristic_t( const problem_t &problem );
  virtual ~zeroHeuristic_t();
  virtual double value( const state_t &state ) { return( 0 ); }
  virtual void statistics( std::ostream &os ) const { }
};


/*******************************************************************************
 *
 * min-min-lrtdp heuristic
 *
 ******************************************************************************/

class minMinLRTDPHeuristic_t : public heuristic_t
{
  const problem_t &relaxation_;
  heuristic_t *heur_;
  hash_t *hash_table_;
  algorithm_t *algorithm_;

public:
  minMinLRTDPHeuristic_t( const problem_t &problem, heuristic_t &heur );
  virtual ~minMinLRTDPHeuristic_t();
  virtual double value( const state_t &state );
  virtual void statistics( std::ostream &os ) const;
};


/*******************************************************************************
 *
 * min-min-ida* heuristic
 *
 ******************************************************************************/

class minMinIDAHeuristic_t : public heuristic_t
{
  const problem_t &relaxation_;
  heuristic_t *heur_;
  hash_t *hash_table_;
  algorithm_t *algorithm_;

public:
  minMinIDAHeuristic_t( const problem_t &problem, heuristic_t &heur, bool useTT );
  virtual ~minMinIDAHeuristic_t();
  virtual double value( const state_t &state );
  virtual void statistics( std::ostream &os ) const;
};


/*******************************************************************************
 *
 * dijkstra heuristic
 *
 ******************************************************************************/

class dijkstraHeuristic_t : public heuristic_t
{
  const problem_t &relaxation_;
  heuristic_t *heur_;
  hash_t *hash_table_;
  algorithm_t *algorithm_;
  unsigned size_;

public:
  dijkstraHeuristic_t( const problem_t &problem, heuristic_t &heur, unsigned size );
  virtual ~dijkstraHeuristic_t();
  virtual double value( const state_t &state );
  virtual void statistics( std::ostream &os ) const;
};


/*******************************************************************************
 *
 * atom-min-1-forward heuristic
 *
 ******************************************************************************/

class atomMin1ForwardHeuristic_t : public heuristic_t
{
  const problem_t &relaxation_;

public:
  atomMin1ForwardHeuristic_t( const problem_t &problem );
  virtual ~atomMin1ForwardHeuristic_t();
  virtual double value( const state_t &state );
  virtual void statistics( std::ostream &os ) const;
};


/*******************************************************************************
 *
 * atom-min-<m>-forward heuristic
 *
 ******************************************************************************/

class atomMinMForwardHeuristic_t : public heuristic_t
{
  size_t m_;
  ushort_t *array_;
  const problem_t &relaxation_;
  enum { OPEN = 0x100, CLOSED = 0x200 };

  class pairINDEX
  {
  public:
    unsigned operator()( const std::pair<unsigned,ushort_t> &pair ) const
    {
      return( pair.second );
    }
  };

public:
  atomMinMForwardHeuristic_t( const problem_t &problem, size_t m );
  virtual ~atomMinMForwardHeuristic_t();
  virtual double value( const state_t &state );
  virtual void statistics( std::ostream &os ) const;
  void subsets( size_t m, const atomList_t &alist, size_t i, ushort_t *array,
		size_t j, std::vector<unsigned> &ssets ) const;
};


/*******************************************************************************
 *
 * atom-min-<m>-backward heuristic
 *
 ******************************************************************************/

class atomMinMBackwardHeuristic_t : public heuristic_t
{
  class pairINDEX
  {
  public:
    unsigned operator()( const std::pair<const atomList_t*,ushort_t> &pair ) const
    {
      return( pair.second );
    }
  };

  class alist_hash_function
  {
  public:
    size_t operator()( const atomList_t *alist ) const
      {
	return( alist->hash_value() );
      }
  };

  class alist_hash_equal
  {
  public:
    bool operator()( const atomList_t *alist1, const atomList_t *alist2 ) const
      {
	return( *alist1 == *alist2 );
      }
  };

  class hash_t : public std::tr1::unordered_map<const atomList_t*,ushort_t,alist_hash_function,alist_hash_equal> { };

public:
  class system_t
  {
    bool goal_;
    bool support_;
    atomListList_t focus_;
    atomListList_t base_;
    atomListList_t frame_;
    atomList_t fmask_;
    atomList_t bmask_;
    hash_t hash_;

    unsigned value_aux( const atomList_t &projection ) const
      {
	hash_t::const_iterator it = hash().find( &projection );
	if( it != hash().end() )
	  {
	    assert( ((*it).second & 0xFF00) == CLOSED );
	    return( (*it).second & 0xFF );
	  }
	else
	  return( gpt::dead_end_value );
      }

  public:
    system_t() : goal_(false), support_(false) { }
    ~system_t() { }

    bool goal( void ) const { return( goal_ ); }
    bool support( void ) const { return( support_ ); }
    void set_goal( bool goal ) { goal_ = goal; }
    void set_support( bool support ) { support_ = support; }
    hash_t& hash( void ) { return( hash_ ); }
    const hash_t& hash( void ) const { return( hash_ ); }
    unsigned value( const atomList_t &alist ) const;

    const atomListList_t& focus( void ) const { return( focus_ ); }
    const atomListList_t& base( void ) const { return( base_ ); }
    const atomListList_t& frame( void ) const { return( frame_ ); }
    void focus_insert( const atomList_t *alist ) { focus_.insert( (atomList_t*) alist ); }
    void base_insert( const atomList_t *alist ) { base_.insert( (atomList_t*) alist ); }
    void frame_insert( const atomList_t *alist ) { frame_.insert( (atomList_t*) alist ); }

    const atomList_t& fmask( void ) const { return( fmask_ ); }
    const atomList_t& bmask( void ) const { return( bmask_ ); }
    void compute_fmask( void );
    void compute_bmask( void );

    void print( std::ostream &os ) const;
  };

private:
  size_t m_;
  const problem_t &relaxation_;
  std::map<const std::string,std::pair<const atomList_t*,bool> > orbits_;
  std::map<const std::string,system_t*> systems_;
  std::map<const std::string,std::vector<system_t*>*> databases_;

  enum { OPEN = 0x100, CLOSED = 0x200 };

public:
  atomMinMBackwardHeuristic_t( const problem_t &problem, size_t m );
  virtual ~atomMinMBackwardHeuristic_t();
  virtual double value( const state_t &state );
  virtual void statistics( std::ostream &os ) const;

  size_t number_databases( void ) const { return( databases_.size() ); }
  unsigned value( const std::vector<system_t*> &database, const atomList_t &alist ) const;
  bool is_cover( const problem_t &problem, const atomList_t &orbit ) const;
  bool prune( const atomList_t &alist, const system_t &system ) const;
  void regression( const atomList_t &alist, ushort_t cost, const system_t &system,
		   std::vector<std::pair<const atomList_t*,ushort_t> > &expansion ) const;
  void compute_mutexes( atomList_t &reachable, std::vector<unsigned> &mutexes,
			std::pair<state_t*,Rational> *initial ) const;
  void compute_orbits( void );
  void compute_systems( void );
  void generate_systems( void );
  void compute_databases( void );
};

inline std::ostream&
operator<<( std::ostream &os, const atomMinMBackwardHeuristic_t::system_t &system )
{
  system.print( os );
  return( os );
}


/*******************************************************************************
 *
 * FF heuristic
 *
 ******************************************************************************/

class ffHeuristic_t : public heuristic_t
{
  const problem_t &relaxation_;
  size_t number_operators_;

  bool nprec_;
  uchar_t *atom_idx_;
  uchar_t *operator_idx_;
  uchar_t *operator_ctr_;
  const deterministicAction_t **operator_ptr_;
  atomList_t *prec_;
  atomList_t *add_;
  atomList_t **goals_;
  atomList_t **true_;
  atomList_t *goal_;

public:
  ffHeuristic_t( const problem_t &problem );
  virtual ~ffHeuristic_t();
  virtual double value( const state_t &state );
  virtual void statistics( std::ostream &os ) const;
  unsigned difficulty( ushort_t op ) const;
  ushort_t select( ushort_t goal, size_t layer ) const;
  void schedule( ushort_t atom, int layer, atomList_t &schedule_ ) const;
};


/*******************************************************************************
 *
 * lookahead heuristic
 *
 ******************************************************************************/

class lookaheadHeuristic_t : public heuristic_t
{
  heuristic_t &heur_;
  unsigned depth_;
  std::pair<state_t*,Rational> **display_;

public:
  lookaheadHeuristic_t( const problem_t &problem, heuristic_t &heur, unsigned depth = 0 );
  virtual ~lookaheadHeuristic_t();
  virtual double value( const state_t &state );
  virtual void statistics( std::ostream &os ) const;
  void set_depth( unsigned depth ) { depth_ = depth; }
  double value( const state_t &state, unsigned depth );
};

#endif // HEURISTICS_H
