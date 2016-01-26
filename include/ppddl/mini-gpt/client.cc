#include "global.h"
#include "client.h"
#include "domains.h"
#include "exceptions.h"
#include "hash.h"
#include "planners.h"
#include "strxml.h"
#include "states.h"
#include <assert.h>

static int 
sessionRequestInfo( XMLNodePtr node, std::ostream &os,
		    int& rounds, float& time, int& turns )
{
  XMLNodePtr settingNode = node->getChild( "setting" );
  if( !settingNode ) return( 0 );

  std::string s;
  if( !dissectNode( settingNode, "rounds", s ) ) return( 0 );
  rounds = atoi( s.c_str() );

  if( !dissectNode( settingNode, "allowed-time", s ) ) return( 0 );
  time = atof( s.c_str() );

  if( !dissectNode( settingNode, "allowed-turns", s ) ) return( 0 );
  turns = atoi( s.c_str() );

  if( gpt::verbosity > 0 )
    {
      os << "<session>: start: rounds = " << rounds << std::endl;
      os << "<session>: start: allowed time = " << time << std::endl;
      os << "<session>: start: turns = " << turns << std::endl;
    }

  return( 1 );
}

static void 
sessionEndInfo( XMLNodePtr node, std::ostream &os, unsigned total_steps )
{
  if( !node || (node->getName() != "end-session") )
    return;
  else
    {
      assert( node->size() == 5 );
      XMLNodePtr id = node->getChild( 0 );
      XMLNodePtr problem = node->getChild( 1 );
      XMLNodePtr rounds = node->getChild( 2 );
      XMLNodePtr goals = node->getChild( 3 );
      XMLNodePtr failed = goals->getChild( 0 );
      XMLNodePtr reached = goals->getChild( 1 );
      XMLNodePtr success = reached->getChild( 0 );
      XMLNodePtr time = reached->getChild( 1 );
      XMLNodePtr metric = node->getChild( 4 );

      unsigned number_success = atoi( success->getText().c_str() );
      os << "<session>: id = " << id->getText() << std::endl;
      os << "<session>: problem = " << problem->getText() << std::endl;
      os << "<session>: rounds = " << rounds->getText() << std::endl;
      os << "<session>: failed = " << failed->getText() << std::endl;
      if( time )
	{
	  os << "<session>: successes = " << number_success << std::endl;
	  os << "<session>: avg. time = " << time->getText() << std::endl;
	}
      else
	os << "<session>: successes = " << number_success << std::endl;
      os << "<session>: avg. steps in successful rounds = "
	 << (number_success==0?0:total_steps/number_success) << std::endl;
      os << "<session>: metric-average = " << metric->getText() << std::endl;
    }
}

static void
roundEndInfo( XMLNodePtr node, std::ostream &os, unsigned &total_steps )
{
  if( !node || (node->getName() != "end-round") )
    return;
  else
    {
      XMLNodePtr round, time, turns;
      assert( (node->size() == 5) || (node->size() == 6) );
      round = node->getChild( 0 );
      if( node->size() == 6 )
	{
	  time = node->getChild( 4 );
	  turns = node->getChild( 5 );
	  total_steps += atoi( turns->getText().c_str() );
	  if( gpt::verbosity >= 50 )
	    os << "<round>: goal reached!!!" << std::endl;
	}
      else
	{
	  time = node->getChild( 3 );
	  turns = node->getChild( 4 );
	}

      if( gpt::verbosity >= 50 )
	{
	  os << "<round>: round = " << round->getText() << std::endl;
	  os << "<round>: time = " << time->getText() << std::endl;
	  os << "<round>: turns = " << turns->getText() << std::endl;
	}
    }
}


/*******************************************************************************
 *
 * XML client
 *
 ******************************************************************************/

XMLClient_t::XMLClient_t( planner_t *planner, const problem_t *problem,
			  std::string name, std::istream& is, std::ostream& os )
  : problem_(problem), planner_(planner)
{
  const state_t *state;
  const action_t *action;

  // create display
  display_ = new std::pair<state_t*,Rational>[DISP_INT_SIZE];
  for( size_t i = 0; i < DISP_INT_SIZE; ++i )
    display_[i].first = new state_t;

  // initialize random planner if necessary
  planner_t *random = 0;
  heuristic_t *zero = 0;
  if( gpt::noise )
    {
      zero = new zeroHeuristic_t( *problem_ );
      random = new plannerRANDOM_t( *problem_, *zero );
    }

  // initiate sesssion
  os << "<session-request>"
     <<  "<name>" << name << "</name>"
     <<  "<problem>" << problem_->name() << "</problem>"
     << "</session-request>" << std::endl;

  // read session parameters
  XMLNodePtr sessionInitNode = 0;
  is >> sessionInitNode;
  if( !sessionRequestInfo( sessionInitNode, std::cout,
			   total_rounds_, round_time_, round_turns_ ) )
    {
      std::cout << "<client>: ERROR: session-request response: " << sessionInitNode << std::endl;
      return;
    }
  delete sessionInitNode;

  unsigned total_steps = 0;
  for( int rounds_left = total_rounds_; rounds_left > 0; --rounds_left )
    {
      // initiate round
      os << "<round-request/>" << std::endl;

      // read round parameters
      XMLNodePtr roundInitNode = 0;
      is >> roundInitNode;
      if( !roundInitNode || (roundInitNode->getName() != "round-init") )
	{
	  std::cout << "<client>: ERROR: round-request response: " << roundInitNode << std::endl;
	  delete roundInitNode;
	  return;
	}
      delete roundInitNode;

#ifndef NDEBUG
      problem_->initial_states( display_ );
      for( size_t i = 0; display_[i].second != Rational( -1 ); ++i )
	{
	  if( gpt::verbosity >= 500 )
	    {
	      std::cout << "<initial-state>: ";
	      display_[i].first->print( std::cout );
	      std::cout << std::endl;
	    }

	  if( !display_[i].first->make_check() ||
	      !(display_[i].first->digest() == display_[i].first->hash_value()) )
	    throw Exception( "inconsistency found in initial state" );
	}
#endif

      // init rount within planner
      planner_->initRound();
      XMLNodePtr response = 0;
      while( true )
	{
	  // read server
	  is >> response;
	  if( !response )
	    {
	      std::cout << "<client>: ERROR: invalid state response: " << response << std::endl;
	      return;
	    }

	  // check if end of round or session
	  if( (response->getName() == "end-round") ||
	      (response->getName() == "end-session") )
	    break;

	  // read state
	  if( !(state = getState( response )) )
	    {
	      std::cout << "<client>: ERROR: invalid state response: " << response << std::endl;
	      return;
	    }

	  // check for dead-end and/or goal
	  if( planner_->dead_end( *state ) )
	    {
	      sendDone( os );
	      std::cout << "    +(</done>)" << std::endl;
	      std::cout << "<client>: state ";
	      state->print( std::cout );
	      std::cout << " is dead-end!!!" << std::endl;
	      continue;
	    }

	  if( problem_->goal().holds( *state ) )
	    {
	      std::cout << "<client>: state ";
	      state->print( std::cout );
	      std::cout << " is goal!!!" << std::endl;
	    }

#ifndef NDEBUG
	  {
	    size_t i = 0;
	    for( ; display_[i].second != Rational( -1 ); ++i )
	      if( *display_[i].first == *state ) break;

	    if( display_[i].second == Rational( -1 ) )
	      {
		std::cout << "state = ";
		state->full_print( std::cout, problem_ );
		std::cout << std::endl;
		std::cout << std::endl;
		for( size_t i = 0; display_[i].second != Rational( -1 ); ++i )
		  {
		    display_[i].first->full_print( std::cout, problem_ );
		    std::cout << std::endl;
		  }
		throw Exception( "unexpected state 1111" );
	      }
	  }
#endif

	  // generate and send action
	  if( gpt::noise && (drand48() < gpt::noise_level) )
	    action = random->decideAction( *state );
	  else
	    action = planner_->decideAction( *state );

	  if( action == NULL )
	    sendDone( os );
	  else
	    sendAction( os, action );

	  if( gpt::verbosity >= 100 )
	    {
	      if( gpt::verbosity >= 150 )
		{
		  std::cout << "<state >:      ";
		  state->full_print( std::cout, problem_ );
		  std::cout << std::endl;
		  std::cout << "<action>: ";
		}

	      if( action )
		{
		  std::cout << "    +";
		  planner_->print( std::cout, *action );
		  std::cout << std::endl;
		}
	      else
		std::cout << "    +(</done>)" << std::endl;
	    }

#ifndef NDEBUG
	  if( action )
	    {
	      problem_->expand( *action, *state, display_ );
	      for( size_t i = 0; display_[i].second != Rational( -1 ); ++i )
		{
		  if( gpt::verbosity >= 500 )
		    {
		      std::cout << "<expansion>: ";
		      display_[i].first->print( std::cout );
		      std::cout << std::endl;
		    }

		  if( !display_[i].first->make_check() ||
		      !(display_[i].first->digest() == display_[i].first->hash_value()) )
		    throw Exception( "inconsistency found in state" );
		}
	    }
#endif

	  delete state;
	}
      planner_->endRound();

      if( response )
	{
	  if( response->getName() == "end-round" )
	    roundEndInfo( response, std::cout, total_steps );
	  else if(response->getName() == "end-session" )
	    {
	      delete response;
	      break;
	    }
	}
      delete response;
    }

  XMLNodePtr endSessionNode = 0;
  is >> endSessionNode;
  if( endSessionNode )
    {
      sessionEndInfo( endSessionNode, std::cout, total_steps );
      delete endSessionNode;
    }  

  // clean random planner
  if( gpt::noise )
    {
      delete zero;
      delete random;
    }
}

XMLClient_t::~XMLClient_t()
{
  for( size_t i = 0; i < DISP_INT_SIZE; ++i )
    delete display_[i].first;
  delete[] display_;
}

const state_t* 
XMLClient_t::getState( XMLNodePtr stateNode )
{
  if( !stateNode || (stateNode->getName() != "state") )
    return( NULL );
  else
    {
      state_t *s = new state_t;
      for( int i = 0; i < stateNode->size(); ++i )
	{
	  XMLNodePtr cn = stateNode->getChild( i );
	  if( cn->getName() == "atom" )
	    {
	      const Atom *a = getAtom( cn );
	      s->add( *a );
	      StateFormula::unregister_use( a );
	    }
	  else if( cn->getName() == "fluent" )
	    {
#ifdef FULL_STATES
	      const Application *a = getApplication( cn );
	      std::string value_str;
	      if( !dissectNode( cn, "value", value_str ) )
		return( NULL );
	      s->add( *a, Rational( value_str.c_str() ) );
#else
	      if( gpt::warning_level > 0 )
		{
		  std::cout << "<client>: WARNING: fluent `";
		  cn->print( std::cout );
		  std::cout << "' received; ignoring." << std::endl;
		}
#endif
	    }
	}
      problem_->complete_state( *s );
      s->make_digest();
      return( s );
    }
}

const Application* 
XMLClient_t::getApplication( XMLNodePtr appNode )
{
  if( !appNode || (appNode->getName() != "fluent") )
    return( NULL );
  else
    {
      // get function name
      std::string function_name;
      if( !dissectNode( appNode, "function", function_name ) )
	return( NULL );

      const Domain& domain = problem_->domain();
      std::pair<Function,bool> p = domain.functions().find_function( function_name );
      if( !p.second ) return( NULL );
      Function function = p.first;

      // get terms
      TermList terms;
      int argIndex = 0;
      for( int i = 0; i < appNode->size(); ++i )
	{
	  XMLNodePtr termNode = appNode->getChild( i );
	  if( !termNode || (termNode->getName() != "term") )
	    continue;

	  Type ptype = domain.functions().parameter( function, argIndex++ );
	  std::string term_name = termNode->getText();
	  std::pair<Object,bool> o = problem_->terms().find_object( term_name );
	  if( o.second )
	    {
	      Type otype = problem_->terms().type( o.first );
	      if( !domain.types().subtype( otype, ptype ) )
		return( NULL );
	    }
	  else
	    {
	      o = domain.terms().find_object( term_name );
	      Type otype = problem_->terms().type( o.first );
	      if( !domain.types().subtype( otype, ptype ) )
		return( NULL );
	    }
	  terms.push_back(o.first);
	}

      if( domain.functions().arity( function ) != terms.size() ) 
	return( NULL );
  
      const Application& a = Application::make_application( function, terms );
      return( &a );
    }
}

const Atom* 
XMLClient_t::getAtom( XMLNodePtr atomNode )
{
  if( !atomNode || (atomNode->getName() != "atom") )
    return( NULL );
  else
    {
      // get predicate name
      std::string predicate_name;
      if( !dissectNode( atomNode, "predicate", predicate_name ) )
	return( NULL );

      const Domain& domain = problem_->domain();
      std::pair<Predicate,bool> p = domain.predicates().find_predicate( predicate_name );
      if( !p.second ) return( NULL );
      Predicate predicate = p.first;

      // get terms
      TermList terms;
      int argIndex = 0;
      for( int i = 0; i < atomNode->size(); ++i )
	{
	  XMLNodePtr termNode = atomNode->getChild( i );
	  if( !termNode || (termNode->getName() != "term") )
	    continue;

	  Type ptype = domain.predicates().parameter( predicate, argIndex++ );
	  std::string term_name = termNode->getText();
	  std::pair<Object,bool> o = problem_->terms().find_object( term_name );
	  if( o.second )
	    {
	      Type otype = problem_->terms().type( o.first );
	      if( !domain.types().subtype( otype, ptype ) )
		return( NULL );
	    }
	  else
	    {
	      o = domain.terms().find_object( term_name );
	      Type otype = problem_->terms().type( o.first );
	      if( !domain.types().subtype( otype, ptype ) )
		return( NULL );
	    }
	  terms.push_back( o.first );
	}

      if( domain.predicates().arity( predicate ) != terms.size() ) 
	return( NULL );

      const Atom& a = Atom::make_atom( predicate, terms );
      return( &a );
    }
}

void 
XMLClient_t::sendDone( std::ostream& os ) const
{
  os << "<done></done>" << std::endl;
}

void 
XMLClient_t::sendAction( std::ostream& os, const action_t *a ) const
{
  os << "<act>";
  a->printXML( os );
  os << "</act>" << std::endl;
}
