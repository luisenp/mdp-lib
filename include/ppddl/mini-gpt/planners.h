#ifndef PLANNERS_H
#define PLANNERS_H

#include "algorithms.h"
#include "actions.h"
#include <iostream>

class problem_t;
class hash_t;
class heuristic_t;


/*******************************************************************************
 *
 * planner (abstract class)
 *
 ******************************************************************************/

class planner_t
{
protected:
  const problem_t &problem_;
  heuristic_t &heur_;
  hash_t *hash_table_;
  algorithm_t *algorithm_;

public:
  planner_t( const problem_t &problem, heuristic_t &heur )
    : problem_(problem), heur_(heur), hash_table_(0), algorithm_(0) { }
  virtual ~planner_t() { }

  enum { VI, LRTDP, ASP, HDPZJ };

  virtual void initRound( void ) { }
  virtual void endRound( void ) { }
  virtual const action_t* decideAction( const state_t &state )
    {
      return( algorithm_->next( state ) );
    }
  bool dead_end( const state_t &state ) const
    {
      if( hash_table_ == 0 )
        return( false );
      else {
        hashEntry_t *entry = hash_table_->get( state );
        return( entry->value() == gpt::dead_end_value );
      }
    }
  void print( std::ostream &os, const action_t &action ) const
    {
      action.print( os );
    }
  virtual void statistics( std::ostream &os, int level ) const = 0;
};


/*******************************************************************************
 *
 * planner RANDOM
 *
 ******************************************************************************/

class plannerRANDOM_t : public planner_t
{
public:
  plannerRANDOM_t( const problem_t &problem, heuristic_t &heur )
    : planner_t(problem,heur)
    {
      algorithm_ = new random_t( problem );
    }
  virtual ~plannerRANDOM_t()
    {
      delete algorithm_;
    }
  virtual void statistics( std::ostream &os, int level ) const { }
};


/*******************************************************************************
 *
 * planner VI
 *
 ******************************************************************************/

class plannerVI_t : public planner_t
{
public:
  plannerVI_t( const problem_t &problem, heuristic_t &heur, double epsilon );
  virtual ~plannerVI_t();
  virtual void statistics( std::ostream &os, int level ) const;
};


/*******************************************************************************
 *
 * planner LRTDP
 *
 ******************************************************************************/

class plannerLRTDP_t : public planner_t
{
public:
  plannerLRTDP_t( const problem_t &problem, heuristic_t &heur, double epsilon );
  virtual ~plannerLRTDP_t();
  virtual void statistics( std::ostream &os, int level ) const;
};


/*******************************************************************************
 *
 * planner ASP
 *
 ******************************************************************************/

class plannerASP_t : public planner_t
{
public:
  plannerASP_t( const problem_t &problem, heuristic_t &heur, int simulations );
  virtual ~plannerASP_t();
  virtual void statistics( std::ostream &os, int level ) const;
};


/*******************************************************************************
 *
 * planner HDP(0,j)
 *
 ******************************************************************************/

class plannerHDPZJ_t : public planner_t
{
  unsigned J_;
  unsigned kappa_;
  hashEntryX_t *last_state_;
  const action_t *last_action_;
  hdpzj_t *hdp_algorithm_;

public:
  plannerHDPZJ_t( const problem_t &problem, heuristic_t &heur, double epsilon, int J );
  virtual ~plannerHDPZJ_t();
  virtual void initRound( void ) { last_state_ = 0; last_action_ = 0; }
  virtual const action_t* decideAction( const state_t &state )
    {
      // update kappa cost
      if( !last_action_ )
	kappa_ = 0;
      else
	kappa_ += hdp_algorithm_->transition_kappa( *last_state_->state(), *last_action_, state );
      
      last_state_ = hdp_algorithm_->get( state );

      if( gpt::verbosity >= 200 )
	{
	  std::cout << "<hdp(0," << J_ << ")>: ";
	  state.print( std::cout );
	  std::cout << ": kappa cost = " << kappa_
		    << ", bits = " << last_state_->bits() << std::endl;
	}

      if( !last_action_ || (!(last_state_->bits() & hdpzj_t::MARK) && (kappa_ >= J_)) )
	{
	  if( gpt::verbosity >= 150 )
	    {
	      std::cout << "<hdp(0," << J_ << ")>: replanning for state ";
	      state.print( std::cout );
	      std::cout << std::endl;
	    }
	  hdp_algorithm_->solve( last_state_ );
	  kappa_ = 0;
	}

      last_action_ = hdp_algorithm_->next( state );
      return( last_action_ );
    }
  virtual void statistics( std::ostream &os, int level ) const;
};

#endif // PLANNERS_H
