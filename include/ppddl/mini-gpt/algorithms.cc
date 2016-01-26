#include "hash.h"
#include "actions.h"
#include "problems.h"
#include "queue.h"
#include "states.h"
#include "algorithms.h"

#include <limits.h>
#include <values.h>
#include <iostream>
#include <math.h>
#include <deque>
#include <set>


/*******************************************************************************
 *
 * value iteration
 *
 ******************************************************************************/

valueIteration_t::valueIteration_t( const problem_t &problem, hash_t &hash_table,
				    double epsilon )
  : algorithm_t(problem), hash_table_(&hash_table), epsilon_(epsilon)
{
  solve();
  if( gpt::verbosity >= 500 )
    std::cout << "<vi>: new" << std::endl;
}

valueIteration_t::~valueIteration_t()
{
  if( gpt::verbosity >= 500 )
    std::cout << "<vi>: deleted" << std::endl;
}

void
valueIteration_t::solve( void )
{
  std::deque<hashEntry_t*>::iterator it;
  std::deque<hashEntry_t*> space;
  state_t::generate_state_space( *problem_, *hash_table_, space );

  if( gpt::verbosity >= 400 )
    {
      std::cout << "<vi>: residual = ";
      std::cout.flush();
    }

  double residual = 1.0 + epsilon_;
  while( residual > epsilon_ )
    {
      residual = 0;
      for( it = space.begin(); it != space.end(); ++it )
	{
	  double oldv = (*it)->value();
	  double newv = hash_table_->bestQValue( *(*it)->state(), *problem_ );
	  (*it)->update( newv );
	  double diff = fabs( oldv - (*it)->value() );
	  residual = (diff>residual?diff:residual);
	}

      if( gpt::verbosity >= 400 )
	{
	  std::cout << residual << " ";
	  std::cout.flush();
	}
    }

  if( gpt::verbosity >= 400 )
    std::cout << std::endl;
}

void
valueIteration_t::statistics( std::ostream &os ) const
{
  os << "<vi>: hash size = " << hash_table_->size() << std::endl;
  os << "<vi>: hash diameter = " << hash_table_->diameter() << std::endl;
  os << "<vi>: hash dimension = " << hash_table_->dimension() << std::endl;
}


/*******************************************************************************
 *
 * lrtdp
 *
 ******************************************************************************/

lrtdp_t::lrtdp_t( const problem_t &problem, hash_t &hash_table, double epsilon )
  : algorithm_t(problem), hash_table_(&hash_table), epsilon_(epsilon)
{
  display_ = new std::pair<state_t*,Rational>[DISP_INT_SIZE];
  for( size_t i = 0; i < DISP_INT_SIZE; ++i )
    display_[i].first = new state_t;

  if( gpt::verbosity >= 500 )
    std::cout << "<lrtdp>: new" << std::endl;
}

lrtdp_t::~lrtdp_t()
{
  for( size_t i = 0; i < DISP_INT_SIZE; ++i )
    delete display_[i].first;
  delete[] display_;

  if( gpt::verbosity >= 500 )
    std::cout << "<lrtdp>: deleted" << std::endl;
}

void
lrtdp_t::trial( hashEntry_t *node )
{
  double value;
  //xxxxx static atomList_t appliedActions;
  std::deque<hashEntry_t*> closed;
  std::deque<hashEntry_t*> Stack;

  if( gpt::verbosity >= 450 )
    std::cout << "<lrtdp>: trial begin:" << std::endl;

  state_t *current = new state_t( *node->state() );
  while( true )
    {
      if( gpt::verbosity >= 450 )
	{
	  std::cout << "<lrtdp>:   node ";
	  node->state()->print( std::cout );
	  std::cout << " = " << node->value() << std::endl;
	}

      if( (node->bits() & SOLVED) || (node->value() >= gpt::dead_end_value) ||
	  problem_->goal().holds( *current ) )
	{
	  if( !(node->bits() & SOLVED) && (gpt::verbosity >= 200) )
	    {
	      std::cout << "<lrtdp>: solved ";
	      node->state()->print( std::cout );
	      std::cout << " with value = " << node->value() << std::endl;
	    }
	  node->set_bits( SOLVED );
	  break;
	}

      Stack.push_front( node );
      //xxxxxx int action = hash_table_->bestAction( *current, *problem_, value, &appliedActions );
      int action = hash_table_->bestAction( *current, *problem_, value );
      if( action == -1 )
	{
	  node->update( gpt::dead_end_value );
	  node->set_bits( SOLVED );
	  if( gpt::verbosity >= 200 )
	    {
	      std::cout << "<lrtdp>: solved ";
	      node->state()->print( std::cout );
	      std::cout << " with value = " << node->value() << std::endl;
	    }
	  break;
	}
      //xxxxxx appliedActions.insert( (ushort_t)action );

      node->update( value );
      problem_->actionsT()[action]->affect( *current );
      problem_->complete_state( *current );
      node = hash_table_->get( *current );
      assert( current->make_check() );

      if( gpt::verbosity >= 450 )
	{
	  std::cout << "<lrtdp>:   action = ";
	  problem_->actionsT()[action]->print( std::cout );
	  std::cout << std::endl;
	}
    }
  delete current;
  //xxxxxxx appliedActions.clear();

  if( gpt::verbosity >= 450 )
    std::cout << "<lrtdp>: trial end." << std::endl;

  while( !Stack.empty() )
    {
      node = Stack.front();
      Stack.pop_front();

      // try labeling
      if( !(node->bits() & SOLVED) )
	{
	  closed.clear();
	  if( checkSolved( node, closed ) )
	    {
	      while( !closed.empty() )
		{
		  closed.front()->set_bits( SOLVED );

		  if( gpt::verbosity >= 200 )
		    {
		      std::cout << "<lrtdp>: solved ";
		      closed.front()->state()->print( std::cout );
		      std::cout << " with value = " << closed.front()->value()
				<< std::endl;
		    }
		  closed.pop_front();
		}
	    }
	  else
	    break;
	}
    }

  for( ; !Stack.empty(); Stack.pop_front() );
}

bool
lrtdp_t::checkSolved( hashEntry_t *node, std::deque<hashEntry_t*> &closed )
{
  double value;
  std::set<hashEntry_t*,std::less<hashEntry_t*> > aux;
  std::deque<hashEntry_t*> open;

  open.clear();
  aux.clear();
  open.push_back( node );
  aux.insert( node );
  bool rv = true;
  while( !open.empty() )
    {
      node = open.front();
      open.pop_front();
      closed.push_front( node );

      int action = hash_table_->bestAction( *node->state(), *problem_, value );
      if( action == -1 )
	{
	  node->update( gpt::dead_end_value );
	  node->set_bits( SOLVED );
	  continue;
	}
      else if( fabs( value - node->value() ) > epsilon_ )
	{
	  rv = false;
	  continue;
	}

      problem_->expand( *problem_->actionsT()[action], *node->state(), display_ );
      for( size_t i = 0; display_[i].second != Rational( -1 ); ++i )
	{
	  hashEntry_t *entry = hash_table_->get( *display_[i].first );
	  if( !(entry->bits() & SOLVED) && (aux.find( entry ) == aux.end()) )
	    {
	      open.push_front( entry );
	      aux.insert( entry );
	    }
	}
    }

  // updated nodes and return
  if( !rv )
    {
      while( !closed.empty() )
	{
	  value = hash_table_->bestQValue( *closed.front()->state(), *problem_ );
	  closed.front()->update( value );
	  closed.pop_front();
	}
    }
  return( rv );
}

void
lrtdp_t::statistics( std::ostream &os ) const
{
  os << "<lrtdp>: hash size = " << hash_table_->size() << std::endl;
  os << "<lrtdp>: hash diameter = " << hash_table_->diameter() << std::endl;
  os << "<lrtdp>: hash dimension = " << hash_table_->dimension() << std::endl;
}


/*******************************************************************************
 *
 * asp
 *
 ******************************************************************************/

asp_t::asp_t( const problem_t &problem, hash_t &hash_table, unsigned simulations )
  : algorithm_t(problem), hash_table_(&hash_table), simulations_(simulations)
{
  if( gpt::verbosity >= 500 )
    std::cout << "<asp>: new" << std::endl;
}

asp_t::~asp_t()
{
  if( gpt::verbosity >= 500 )
    std::cout << "<asp>: deleted" << std::endl;
}

void
asp_t::trial( hashEntry_t *node )
{
  double value;
  unsigned steps = 0;
  state_t *current = new state_t( *node->state() );
  while( (steps < gpt::cutoff) && (node->value() < gpt::dead_end_value) &&
	 !problem_->goal().holds( *current ) )
    {
      ++steps;
      int action = hash_table_->bestAction( *current, *problem_, value );
      if( action == -1 )
	{
	  node->update( gpt::dead_end_value );
	  continue;
	}

      node->update( value );
      problem_->actionsT()[action]->affect( *current );
      problem_->complete_state( *current );
      node = hash_table_->get( *current );
    }
  delete current;
}

void
asp_t::statistics( std::ostream &os ) const
{
  os << "<asp>: hash size = " << hash_table_->size() << std::endl;
  os << "<asp>: hash diameter = " << hash_table_->diameter() << std::endl;
  os << "<asp>: hash dimension = " << hash_table_->dimension() << std::endl;
}


/*******************************************************************************
 *
 * hdp(0,j)
 *
 ******************************************************************************/

hdpzj_t::hdpzj_t( const problem_t &problem, hash_t &hash_table, double epsilon,
		  unsigned J )
  : algorithm_t(problem), hash_table_(&hash_table), epsilon_(epsilon), J_(J)
{
  logbase_ = log( 2.0 );
  hash_table_->set_extended();

  if( gpt::verbosity >= 500 )
    std::cout << "<hdp(0," << J_ << ")>: new" << std::endl;
}

hdpzj_t::~hdpzj_t()
{
  if( gpt::verbosity >= 500 )
    std::cout << "<hdp(0," << J_ << ")>: deleted" << std::endl;
}

unsigned
hdpzj_t::transition_kappa( const state_t &state, const action_t &action,
			   const state_t &nstate ) const
{
  unsigned result = 0;
  unsigned normz = UINT_MAX;
  const stateProbList_t *expansion;
  stateProbList_t::const_iterator si;

  expansion = &problem_->expand( action, state );

  for( si = expansion->begin(); si != expansion->end(); ++si )
    normz = MIN(normz,kappa( (*si).second.double_value() ));

  for( si = expansion->begin(); si != expansion->end(); ++si )
    if( *(*si).first == nstate )
      {
	result = kappa( (*si).second.double_value() ) - normz;
	break;
      }

  for( si = expansion->begin(); si != expansion->end(); ++si )
    delete (*si).first;
  delete expansion;

  return( result );
}

bool
hdpzj_t::dfs( hashEntryX_t *node, int cost, unsigned &index, hashEntryX_t* &top,
	      std::deque<hashEntryX_t*> &visited )
{
  // base cases
  if( (node->bits() & MARK) || problem_->goal().holds( *node->state() ) )
    {
      node->set_bits( node->bits() | MARK );
      return( false );
    }

  // if residual > epsilon, update and return
  double value;
  int action = hash_table_->bestAction( *node->state(), *problem_, value );
  if( action == -1 )
    {
      node->update( gpt::dead_end_value );
      node->set_bits( node->bits() | MARK );
      return( false );
    }
  else if( fabs( value - node->value() ) > epsilon_ )
    {
      do {
	node->update( value );
	value = hash_table_->bestQValue( *node->state(), *problem_ );
      } while( fabs( value - node->value() ) > epsilon_ );
      return( true );
    }

  // visit node, expand and order
  visited.push_front( node );
  node->set_elink( top );
  top = node;
  node->set_idx( index );
  node->set_low( index );
  node->set_bits( node->bits() | ACTIVE );
  ++index;

  // recursive call
  const stateProbList_t *expansion;
  stateProbList_t::const_iterator si;
  expansion = &problem_->expand( *problem_->actionsT()[action], *node->state() );

  unsigned normz = UINT_MAX;
  for( si = expansion->begin(); si != expansion->end(); ++si )
    normz = MIN(normz,kappa( (*si).second.double_value() ));

  bool flag = false;
  for( si = expansion->begin(); si != expansion->end(); ++si )
    {
      int ncost = cost + kappa( (*si).second.double_value() ) - normz;
      if( ncost == 0 )
	{
	  hashEntryX_t *nnode = get( *(*si).first );
	  if( nnode->idx() == USHORT_MAX )
	    {
	      bool rv = dfs( nnode, ncost, index, top, visited );
	      node->set_low( MIN(node->low(),nnode->low()) );
	      flag = flag || rv;
	    }
	  else if( nnode->bits() & ACTIVE )
	    {
	      node->set_low( MIN(node->low(),nnode->idx()) );
	    }
	}
      delete (*si).first;
    }
  delete expansion;

  // try to label and update graph
  if( flag )
    {
      double value = hash_table_->bestQValue( *node->state(), *problem_ );
      do {
	node->update( value );
	value = hash_table_->bestQValue( *node->state(), *problem_ );
      } while( fabs( value - node->value() ) > epsilon_ );
    }
  else if( node->idx() == node->low() )
    {
      while( top != node )
	{
	  if( gpt::verbosity >= 300 )
	    {
	      std::cout << "<hdpzj_t>: value for ";
	      top->state()->print( std::cout );
	      std::cout << " = " << top->value() << std::endl;
	    }
	  top->set_bits( top->bits() | MARK );
	  top->set_bits( top->bits() & ~ACTIVE );
	  top = top->elink();
	}

      if( gpt::verbosity >= 300 )
	{
	  std::cout << "<hdpzj_t>: value for ";
	  top->state()->print( std::cout );
	  std::cout << " = " << top->value() << std::endl;
	}
      top->set_bits( top->bits() | MARK );
      top->set_bits( top->bits() & ~ACTIVE );
      top = top->elink();
    }
  return( flag );
}

void
hdpzj_t::solve( hashEntryX_t* entry )
{
  std::deque<hashEntryX_t*> visited;

  unsigned index = 0;
  while( !(entry->bits() & MARK) )
    {
      hashEntryX_t *top = NULL;
      dfs( entry, 0, index, top, visited );

      // clean visited
      while( !visited.empty() )
	{
	  top = visited.front();
	  top->set_bits( entry->bits() & MARK );
	  top->set_idx( USHORT_MAX );
	  top->set_low( USHORT_MAX );
	  top->set_elink( NULL );
	  visited.pop_front();
	}
    }
}

void
hdpzj_t::statistics( std::ostream &os ) const
{
  os << "<hdp(0," << J_ << ")>: hash size = " << hash_table_->size() << std::endl;
  os << "<hdp(0," << J_ << ")>: hash diameter = " << hash_table_->diameter() << std::endl;
  os << "<hdp(0," << J_ << ")>: hash dimension = " << hash_table_->dimension() << std::endl;
}


/*******************************************************************************
 *
 * IDA
 *
 ******************************************************************************/

IDA_t::IDA_t( const problem_t &problem, hash_t &hash_table, bool useTT )
  : algorithm_t(problem), hash_table_(&hash_table), useTT_(useTT)
{
  if( gpt::verbosity >= 500 )
    std::cout << "<ida*>: new" << std::endl;
}

IDA_t::~IDA_t()
{
  if( gpt::verbosity >= 500 )
    std::cout << "<ida*>: deleted" << std::endl;
}

double
IDA_t::ida( const state_t &state, double cost, double threshold, bool &found )
{
  if( problem_->goal().holds( state ) )
    {
      found = true;
      return( cost );
    }
  else if( cost + hash_table_->value( state ) > threshold )
    {
      return( cost + hash_table_->value( state ) );
    }
  else
    {
      const stateProbList_t *expansion;
      actionList_t::const_iterator ai;
      double new_threshold = gpt::dead_end_value;
      for( ai = problem_->actionsT().begin(); ai != problem_->actionsT().end(); ++ai )
	if( (*ai)->enabled( state ) )
	  {
	    expansion = &problem_->expand( **ai, state );
	    assert( expansion->size() == 1 );
	    double tmp = ida( *(*expansion->begin()).first,
			      cost + (*ai)->cost( state ), threshold, found );
	    delete (*expansion->begin()).first;
	    if( found )
	      {
		delete expansion;
		new_threshold = tmp;
		break;
	      }
	    new_threshold = MIN(new_threshold,tmp);
	    delete expansion;
	  }
      return( new_threshold );
    }
}

double
IDA_t::ida( hashEntry_t *node, double cost, double threshold, bool &found )
{
  if( problem_->goal().holds( *node->state() ) )
    {
      found = true;
      return( cost );
    }
  else if( cost + node->value() > threshold )
    {
      return( cost + node->value() );
    }
  else if( cost + node->value() >= gpt::dead_end_value )
    {
      return( gpt::dead_end_value );
    }
  else
    {
      const stateProbList_t *expansion;
      actionList_t::const_iterator ai;
      double new_threshold = gpt::dead_end_value;

      for( ai = problem_->actionsT().begin(); ai != problem_->actionsT().end(); ++ai )
	if( (*ai)->enabled( *node->state() ) )
	  {
	    expansion = &problem_->expand( **ai, *node->state() );
	    assert( expansion->size() == 1 );
	    hashEntry_t *entry = hash_table_->get( *(*expansion->begin()).first );
	    double tmp = ida( entry,
			      cost + (*ai)->cost( *entry->state() ), threshold, found );
	    delete (*expansion->begin()).first;
	    if( found )
	      {
		delete expansion;
		new_threshold = tmp;
		break;
	      }
	    new_threshold = MIN(new_threshold,tmp);
	    delete expansion;
	  }

      node->update( MAX(node->value(),new_threshold-cost) );
      if( found ) node->set_bits( SOLVED );
      return( new_threshold );
    }
}

double
IDA_t::solve( const state_t &state )
{
  bool found = false;
  double threshold = hash_table_->value( state );
  while( !found && (threshold < gpt::dead_end_value) )
    threshold = ida( state, 0, threshold, found );
  return( threshold );
}

void
IDA_t::solve( hashEntry_t *node )
{
  bool found = false;
  double threshold = node->value();
  while( !found && (threshold < gpt::dead_end_value) )
    threshold = ida( node, 0, threshold, found );
  node->update( threshold );
  node->set_bits( SOLVED );
  assert( node->bits() & SOLVED );
}

void
IDA_t::statistics( std::ostream &os ) const
{
  if( useTT_ )
    {
      os << "<ida*>: hash size = " << hash_table_->size() << std::endl;
      os << "<ida*>: hash diameter = " << hash_table_->diameter() << std::endl;
      os << "<ida*>: hash dimension = " << hash_table_->dimension() << std::endl;
    }
}


/*******************************************************************************
 *
 * bounded Dijkstra
 *
 ******************************************************************************/

class pqCMP
{
public:
  unsigned operator()( const hashEntry_t *entry ) const
    {
      return( (unsigned)entry->value() );
    }
};

boundedDijkstra_t::boundedDijkstra_t( const problem_t &problem, hash_t &hash_table, 
				      unsigned size )
  : algorithm_t(problem), hash_table_(&hash_table), size_(size)
{
  std::deque<hashEntry_t*> space;
  priorityQueue_t<hashEntry_t*,pqCMP> PQ;
  std::map<hashEntry_t*,ushort_t> map;

  if( true || gpt::verbosity >= 500 )
    std::cout << "<bounded-dijkstra-" << size_ << ">: new" << std::endl;

  // generate space and detect goal states
  state_t::generate_state_space( *problem_, *hash_table_, space );
  for( std::deque<hashEntry_t*>::iterator it = space.begin(); it != space.end(); ++it )
    if( problem_->goal().holds( *(*it)->state() ) )
      {
	(*it)->update( 0 );
	(*it)->set_bits( OPEN );
	PQ.push( (*it) );

	if( gpt::verbosity >= 500 )
	  {
	    std::cout << "<bounded-dijkstra-" << size_ << ">: goal state ";
	    (*it)->state()->print( std::cout );
	    std::cout << std::endl;
	  }
      }

  // apply dijkstra until number of states == size_ (size_ = 0 means "all states")
  size_t count = 0;
  while( !PQ.empty() )
    {
      hashEntry_t *entry = PQ.top();
      entry->set_bits( CLOSED );
      PQ.pop();
      ++count;

      if( true || gpt::verbosity >= 450 )
	{
	  std::cout << "<bounded-dijkstra-" << size_ << ">: state ";
	  entry->state()->print( std::cout );
	  std::cout << " = " << entry->value() << std::endl;
	}

      // check if done
      if( count == size_ ) break;

      // expand (problem_ is a weak relaxation)
      actionList_t::const_iterator ai = problem_->actionsT().begin();
      for( ; ai != problem_->actionsT().end(); ++ai )
	{
	  const deterministicAction_t *action = (const deterministicAction_t*)(*ai);
	  double cost = entry->value() + 1; // 1 should be (*ai)'s cost

	  // check add-list intersects state
	  size_t i;
	  for( i = 0; i < action->effect().s_effect().add_list().size(); ++i )
	    if( entry->state()->holds( action->effect().s_effect().add_list().atom(i) ) )
	      break;
	  if( i == action->effect().s_effect().add_list().size() )
	    continue;

	  // check del-list does not intersect state
	  for( i = 0; i < action->effect().s_effect().del_list().size(); ++i )
	    if( entry->state()->holds( action->effect().s_effect().del_list().atom(i) ) )
	      break;
	  if( i < action->effect().s_effect().del_list().size() )
	    continue;

	  // regression state
	  state_t state( *entry->state() );

	  // remove add-list
	  for( i = 0; i < action->effect().s_effect().add_list().size(); ++i )
	    state.clear( action->effect().s_effect().add_list().atom( i ) );

	  // add positive preconditions and check negative preconditions
	  assert( (*ai)->precondition().size() == 1 );
	  for( i = 0; i < (*ai)->precondition().atom_list( 0 ).size(); ++i )
	    if( (*ai)->precondition().atom_list( 0 ).atom( i ) % 2 == 0 )
	      state.add( (*ai)->precondition().atom_list( 0 ).atom( i ) );
	    else if( state.holds( (*ai)->precondition().atom_list( 0 ).atom( i ) - 1 ) )
	      break;
	  if( i < (*ai)->precondition().atom_list( 0 ).size() )
	    continue;

	  // check we have a valid state
	  hashEntry_t *tmp = hash_table_->find( state );
	  if( tmp == NULL ) continue;

	  // insert into priority queue
	  if( !tmp->bits() || ((tmp->bits() & OPEN) && (cost < tmp->value())) )
	    {
#if 0
	      std::cout << "state ";
	      state.print( std::cout );
	      std::cout << " comes from state ";
	      p.first->state()->print( std::cout );
	      std::cout << " with action ";
	      (*ai)->print( std::cout );
	      std::cout << std::endl;
#endif

	      if( tmp->bits() & OPEN ) PQ.remove( tmp );
	      tmp->update( cost );
	      tmp->set_bits( OPEN );
	      PQ.push( tmp );
	    }
	}
    }
}

boundedDijkstra_t::~boundedDijkstra_t()
{
  if( gpt::verbosity >= 500 )
    std::cout << "<bounded-dijkstra-" << size_ << ">: deleted" << std::endl;
}

double
boundedDijkstra_t::ida( hashEntry_t *node, double cost, double threshold, bool &found )
{
  if( problem_->goal().holds( *node->state() ) )
    {
      found = true;
      return( cost );
    }
  else if( cost + node->value() > threshold )
    {
      return( cost + node->value() );
    }
  else if( cost + node->value() >= gpt::dead_end_value )
    {
      return( gpt::dead_end_value );
    }
  else
    {
      const stateProbList_t *expansion;
      actionList_t::const_iterator ai;
      double new_threshold = gpt::dead_end_value;

      for( ai = problem_->actionsT().begin(); ai != problem_->actionsT().end(); ++ai )
	if( (*ai)->enabled( *node->state() ) )
	  {
	    expansion = &problem_->expand( **ai, *node->state() );
	    assert( expansion->size() == 1 );
	    hashEntry_t *entry = hash_table_->get( *(*expansion->begin()).first );
	    double tmp = ida( entry, cost + (*ai)->cost( *entry->state() ),
			      threshold, found );
	    delete (*expansion->begin()).first;
	    if( found )
	      {
		new_threshold = tmp;
		delete expansion;
		break;
	      }
	    new_threshold = MIN(new_threshold,tmp);
	    delete expansion;
	  }

      node->update( MAX(node->value(),new_threshold-cost) );
      if( found ) node->set_bits( CLOSED );
      return( new_threshold );
    }
}

void
boundedDijkstra_t::solve( hashEntry_t *node )
{
  bool found = false;
  double threshold = node->value();

  while( !found && (threshold < gpt::dead_end_value) )
    threshold = ida( node, 0, threshold, found );
  node->update( threshold );
}

void
boundedDijkstra_t::statistics( std::ostream &os ) const
{
  os << "<bounded-dijkstra-" << size_ << ">: hash size = "
     << hash_table_->size() << std::endl;
  os << "<bounded-dijkstra-" << size_ << ">: hash diameter = "
     << hash_table_->diameter() << std::endl;
  os << "<bounded-dijkstra-" << size_ << ">: hash dimension = "
     << hash_table_->dimension() << std::endl;
}
