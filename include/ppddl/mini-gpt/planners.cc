#include "global.h"
#include "hash.h"
#include "planners.h"
#include "states.h"
#include <assert.h>


/*******************************************************************************
 *
 * planner VI
 *
 ******************************************************************************/

plannerVI_t::plannerVI_t( const problem_t &problem, heuristic_t &heur, double epsilon )
  : planner_t(problem,heur)
{
  hash_table_ = new hash_t( gpt::initial_hash_size, heur_ );
  algorithm_ = new valueIteration_t( problem, *hash_table_, epsilon );
}

plannerVI_t::~plannerVI_t()
{
  delete hash_table_;
  delete algorithm_;
}

void
plannerVI_t::statistics( std::ostream &os, int level ) const
{
  if( level > 0 )
    algorithm_->statistics( os );

  if( level >= 300 )
    hash_table_->print( os, problem_ );
}


/*******************************************************************************
 *
 * planner LRTDP
 *
 ******************************************************************************/

plannerLRTDP_t::plannerLRTDP_t( const problem_t &problem, heuristic_t &heur, double epsilon )
  : planner_t(problem,heur)
{
  hash_table_ = new hash_t( gpt::initial_hash_size, heur_ );
  algorithm_ = new lrtdp_t( problem, *hash_table_, epsilon );
}

plannerLRTDP_t::~plannerLRTDP_t()
{
  delete algorithm_;
  delete hash_table_;
}

void
plannerLRTDP_t::statistics( std::ostream &os, int level ) const
{
  if( level > 0 )
    algorithm_->statistics( os );

  if( level >= 300 )
    hash_table_->print( os, problem_ );
}


/*******************************************************************************
 *
 * planner ASP
 *
 ******************************************************************************/

plannerASP_t::plannerASP_t( const problem_t &problem, heuristic_t &heur, int simulations )
  : planner_t(problem,heur)
{
  hash_table_ = new hash_t( gpt::initial_hash_size, heur_ );
  algorithm_ = new asp_t( problem, *hash_table_, simulations );
}

plannerASP_t::~plannerASP_t()
{
  delete hash_table_;
  delete algorithm_;
}

void
plannerASP_t::statistics( std::ostream &os, int level ) const
{
  if( level > 0 )
    algorithm_->statistics( os );

  if( level >= 300 )
    hash_table_->print( os, problem_ );
}


/*******************************************************************************
 *
 * planner HDP(0,j)
 *
 ******************************************************************************/

plannerHDPZJ_t::plannerHDPZJ_t( const problem_t &problem, heuristic_t &heur, 
				double epsilon, int J )
  : planner_t(problem,heur), J_(J)
{
  hash_table_ = new hash_t( gpt::initial_hash_size, heur_ );
  hdp_algorithm_ = new hdpzj_t( problem, *hash_table_, epsilon, J_ );
}

plannerHDPZJ_t::~plannerHDPZJ_t()
{
  delete hash_table_;
  delete hdp_algorithm_;
}

void
plannerHDPZJ_t::statistics( std::ostream &os, int level ) const
{
  if( level > 0 )
    hdp_algorithm_->statistics( os );

  if( level >= 300 )
    hash_table_->print( os, problem_ );
}
