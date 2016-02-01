#include "actions.h"
#include "exceptions.h"
#include "problems.h"
#include "domains.h"
#include "formulas.h"
#include "states.h"

#include <assert.h>
#include <sstream>
#include <stack>


/*******************************************************************************
 *
 * action schema
 *
 ******************************************************************************/

ActionSchema::ActionSchema( const std::string& name )
  : name_(name), precondition_(&StateFormula::TRUE), effect_(new ConjunctiveEffect())
{
  StateFormula::register_use( precondition_ );
}

ActionSchema::~ActionSchema()
{
  StateFormula::unregister_use( precondition_ );
  Effect::unregister_use( effect_ );
}

void
ActionSchema::set_precondition( const StateFormula& precondition )
{
  if( &precondition != precondition_ )
    {
      StateFormula::unregister_use( precondition_ );
      precondition_ = &precondition;
      StateFormula::register_use( precondition_ );
    }
}

void
ActionSchema::set_effect( const Effect& effect )
{
  if( &effect != effect_ )
    {
      Effect::unregister_use( effect_ );
      effect_ = &effect;
      Effect::register_use( effect_ );
    }
}

void
ActionSchema::instantiations( ActionList& actions, const problem_t& problem ) const
{
  size_t n = arity();
  if( n == 0 )
    {
      SubstitutionMap subst;
      const StateFormula& precond = precondition().instantiation( subst, problem );
      if( !precond.contradiction() )
	actions.push_back( &instantiation( subst, problem, precond ) );
      else
	StateFormula::unregister_use( &precond );
    }
  else
    {
      SubstitutionMap args;
      std::vector<ObjectList> arguments( n, ObjectList() );
      std::vector<ObjectList::const_iterator> next_arg;
      for( size_t i = 0; i < n; ++i )
	{
	  problem.compatible_objects( arguments[i],
				      problem.domain().terms().type( parameter(i) ) );
	  if( arguments[i].empty() ) return;
	  next_arg.push_back( arguments[i].begin() );
	}

      std::stack<const StateFormula*> preconds;
      preconds.push( precondition_ );
      StateFormula::register_use( preconds.top() );
      for( size_t i = 0; i < n; )
	{
	  SubstitutionMap pargs;
	  args.insert( std::make_pair( parameter(i), *next_arg[i] ) );
	  pargs.insert( std::make_pair( parameter(i), *next_arg[i] ) );
	  const StateFormula& precond = preconds.top()->instantiation( pargs, problem );
	  preconds.push( &precond );

	  if( (i + 1 == n) || precond.contradiction() )
	    {
	      if( !precond.contradiction() )
		{
		  StateFormula::register_use( preconds.top() );
		  actions.push_back( &instantiation( args, problem, precond ) );
		}

	      for( int j = i; j >= 0; --j )
		{
		  StateFormula::unregister_use( preconds.top() );
		  preconds.pop();
		  args.erase( parameter( j ) );
		  ++next_arg[j];
		  if( next_arg[j] == arguments[j].end() )
		    {
		      if( j == 0 )
			{
			  i = n;
			  break;
			}
		      else
			next_arg[j] = arguments[j].begin();
		    }
		  else
		    {
		      i = j;
		      break;
		    }
		}
	    }
	  else
	    ++i;
	}

      while( !preconds.empty() )
	{
	  StateFormula::unregister_use( preconds.top() );
	  preconds.pop();
	}
    }
}

const Action&
ActionSchema::instantiation( const SubstitutionMap& subst, const problem_t& problem,
			     const StateFormula& precond ) const
{
  Action *action = new Action( name() );

  for( size_t i = 0; i < arity(); ++i )
    {
      SubstitutionMap::const_iterator si = subst.find( parameter( i ) );
      action->add_argument( (*si).second );
    }

  action->set_precondition( precond );
  StateFormula::unregister_use( &precond );
  const Effect *eff = &effect().instantiation( subst, problem );
  action->set_effect( *eff );
  Effect::unregister_use( eff );

  return( *action );
}

void
ActionSchema::print( std::ostream& os, const PredicateTable& predicates,
		     const FunctionTable& functions,
		     const TermTable& terms ) const
{
  os << "  " << name();
  os << std::endl << "    parameters:";
  for( VariableList::const_iterator vi = parameters_.begin(); vi != parameters_.end(); ++vi )
    {
      os << ' ';
      terms.print_term( os, *vi );
    }
  os << std::endl << "    precondition: ";
  precondition().print( os, predicates, functions, terms );
  os << std::endl << "    effect: ";
  effect().print( os, predicates, functions, terms );
}

void
ActionSchema::analyze( PredicateTable &predicates, TermTable &terms,
		       std::map<const StateFormula*,const Atom*> &hash ) const
{
  if( gpt::verbosity >= 350 )
    std::cout << "analyzing schema for action `" << name() << "'" << std::endl;
  precondition().analyze( predicates, terms, hash );
  effect().analyze( predicates, terms, hash );
}

const ActionSchema&
ActionSchema::rewrite( std::map<const StateFormula*,const Atom*> &hash ) const
{
  ActionSchema *action = new ActionSchema( name() );
  for( size_t i = 0; i < arity(); ++i )
    action->add_parameter( parameter( i ) );
  action->set_precondition( precondition().rewrite( hash ) );
  action->set_effect( effect().rewrite( hash ) );
  return( *action );
}


/*******************************************************************************
 *
 * action
 *
 ******************************************************************************/

Action::Action( const std::string& name )
  : ref_count_(0), name_(name), precondition_(&StateFormula::TRUE),
    effect_(new ConjunctiveEffect())
{
  notify( this, "Action::Action(std::string&)" );
  Action::register_use( this );
  StateFormula::register_use( precondition_ );
}

Action::~Action()
{
  assert( ref_count_ == 0 );
  StateFormula::unregister_use( precondition_ );
  Effect::unregister_use( effect_ );
}

void
Action::set_precondition( const StateFormula& precondition )
{
  if( &precondition != precondition_ )
    {
      StateFormula::unregister_use( precondition_ );
      precondition_ = &precondition;
      StateFormula::register_use( precondition_ );
    }
}

void
Action::set_effect( const Effect& effect )
{
  if( &effect != effect_ )
    {
      Effect::unregister_use( effect_ );
      effect_ = &effect;
      Effect::register_use( effect_ );
    }
}

const Action&
Action::flatten( const problem_t &problem ) const
{
  Action *action = new Action( name() );

  for( ObjectList::const_iterator oi = arguments_.begin(); oi != arguments_.end(); )
    action->add_argument( *oi++ );

  const StateFormula *prec = &precondition().flatten( false );
  action->set_precondition( *prec );
  StateFormula::unregister_use( prec );

  const Effect *eff = &effect().flatten();
  action->set_effect( *eff );
  Effect::unregister_use( eff );

  return( *action );
}

const action_t&
Action::translate( const problem_t &problem ) const
{
  // generate names
  std::ostringstream ost, ostXML;
  ost << "(" << name();
  ostXML << "<action><name>" << name() << "</name>";
  for( ObjectList::const_iterator oi = arguments_.begin(); oi != arguments_.end(); ++oi )
    {
      ost << ' ';
      problem.terms().print_term( ost, *oi );
      ostXML << "<term>";
      problem.terms().print_term( ostXML, *oi );
      ostXML << "</term>";
    }
  ost << ")";
  ostXML << "</action>";

  const ProbabilisticEffect *peff =
    dynamic_cast<const ProbabilisticEffect*>(&effect());
  if( peff != NULL )
    {
      probabilisticAction_t *action = new probabilisticAction_t( ost.str(), ostXML.str() );
      precondition().translate( action->precondition() );
      peff->translate( action->effect() );

      if( action->effect().size() > 1 )
	return( *action );
      else
	{
	  assert( action->probability( 0 ) == 1 );
	  deterministicAction_t *daction =
	    new deterministicAction_t( ost.str(), ostXML.str() );
	  daction->precondition() = action->precondition();
	  daction->effect() = action->effect( 0 );
	  action_t::unregister_use( action );
	  return( *daction );
	}
    }
  else
    {
      deterministicAction_t *action = new deterministicAction_t( ost.str(), ostXML.str() );
      precondition().translate( action->precondition() );
      effect().translate( action->effect().s_effect(), action->effect_.c_effect() );
      return( *action );
    }
}

bool
Action::enabled( const state_t& state ) const
{
  return( precondition().holds( state ) );
}

void
Action::affect( state_t& state ) const
{
  AtomList adds;
  AtomList deletes;
  AssignmentList assig;
  effect().state_change( adds, deletes, assig, state );

  for( AtomList::const_iterator ai = deletes.begin(); ai != deletes.end(); ++ai )
    state.clear( **ai );

  for( AtomList::const_iterator ai = adds.begin(); ai != adds.end(); ++ai )
    state.add( **ai );

  for( AssignmentList::const_iterator ai = assig.begin(); ai != assig.end(); ++ai )
    (*ai)->affect( state );

  state.make_digest();
  assert( state.make_check() );
}

const stateProbList_t&
Action::expand( const state_t &state ) const
{
  state_t *nstate = new state_t( state );
  const stateProbList_t state_list( nstate, 1 );
  const stateProbList_t *result = &effect().expand( state_list );
  delete nstate;
  return( *result );
}

void
Action::print_full( std::ostream& os, const PredicateTable& predicates,
		    const FunctionTable& functions,
		    const TermTable& terms ) const
{
  os << "(action (" << name();
  for( ObjectList::const_iterator oi = arguments_.begin(); oi != arguments_.end(); ++oi )
    {
      os << ' ';
      terms.print_term( os, *oi );
    }
  os << ")" << std::endl << "        (prec ";
  precondition().print( os, predicates, functions, terms );
  os << ")" << std::endl << "        (eff ";
  effect().print( os, predicates, functions, terms );
  os << "))";
}

void
Action::print( std::ostream& os, const TermTable& terms ) const
{
  os << '(' << name();
  for( ObjectList::const_iterator oi = arguments_.begin(); oi != arguments_.end(); ++oi )
    {
      os << ' ';
      terms.print_term( os, *oi );
    }
  os << ')';
}

void
Action::printXML( std::ostream& os, const TermTable& terms ) const
{
  os << "<action><name>" << name() << "</name>";
  for( ObjectList::const_iterator oi = arguments_.begin(); oi != arguments_.end(); ++oi )
    {
      os << "<term>";
      terms.print_term( os, *oi );
      os << "</term>";
    }
  os << "</action>";
}


/*******************************************************************************
 *
 * conditional effect list
 *
 ******************************************************************************/

bool
conditionalEffectList_t::affect( state_t &resulting_state,
                                 state_t &original_state,
                                 bool nprec ) const
{
  bool rv = false;
  for( size_t i = 0; i < size(); ++i )
    rv = effect( i ).affect( resulting_state, original_state, nprec ) || rv;
  return( rv );
}

void
conditionalEffectList_t::print( std::ostream &os ) const
{
  for( size_t i = 0; i < size(); ++i )
    {
      effect( i ).print( os );
      if( i + 1 < size() ) os << " ";
    }
}

bool
conditionalEffectList_t::operator==( const conditionalEffectList_t &clist ) const
{
  for( size_t i = 0; i < size(); ++i )
    if( !clist.find( effect( i ) ) )
      return( false );
  for( size_t i = 0; i < clist.size(); ++i )
    if( !find( clist.effect( i ) ) )
      return( false );
  return( true );
}

conditionalEffectList_t&
conditionalEffectList_t::operator=( const conditionalEffectList_t &clist )
{
  clear();
  for( size_t i = 0; i < clist.size(); ++i )
    {
      conditionalEffect_t *ceffect = new conditionalEffect_t;
      *ceffect = clist.effect( i );
      insert( ceffect );
    }
  return( *this );
}


/*******************************************************************************
 *
 * probabilistic effect list
 *
 ******************************************************************************/

bool
probabilisticEffectList_t::affect( state_t &resulting_state,
                                   state_t &original_state,
                                   bool nprec ) const
{
  double r = drand48();
  double sum = 0;
  for( size_t i = 0; i < size(); ++i )
    {
      sum += effect( i ).probability().double_value();
      if( r < sum )
	return( effect( i ).affect( resulting_state, original_state, nprec ) );
    }
  return( false );
}

void
probabilisticEffectList_t::print( std::ostream &os ) const
{
  os << "(probabilistic";
  for( size_t i = 0; i < size(); ++i )
    {
      os << " ";
      effect( i ).print( os );
    }
  os << ")";
}

probabilisticEffectList_t&
probabilisticEffectList_t::operator=( const probabilisticEffectList_t &plist )
{
  clear();
  for( size_t i = 0; i < plist.size(); ++i )
    {
      probabilisticEffect_t *peffect =
	new probabilisticEffect_t( plist.effect( i ).probability() );
      *peffect = plist.effect( i );
      insert( peffect );
    }
  return( *this );
}


/*******************************************************************************
 *
 * strips effect
 *
 ******************************************************************************/

bool
stripsEffect_t::affect( state_t &resulting_state,
                        state_t &original_state,
                        bool nprec ) const
{
  bool rv = false;
  for( size_t i = 0; i < add_list().size(); ++i )
    rv = resulting_state.add( add_list().atom( i ) ) || rv;
  for( size_t i = 0; i < del_list().size(); ++i )
    rv = resulting_state.clear( del_list().atom( i ) ) || rv;
  return( rv );
}

void
stripsEffect_t::collect_prec_atoms( atomList_t &atoms ) const
{
}

void
stripsEffect_t::collect_add_atoms( atomList_t &atoms ) const
{
  atoms.insert( add_list() );
}

void
stripsEffect_t::collect_del_atoms( atomList_t &atoms ) const
{
  atoms.insert( del_list() );
}

void
stripsEffect_t::print( std::ostream &os ) const
{
  if( add_list().size() > 0 )
    {
      os << "(add ";
      add_list().print( os );
      os << ")";
    }

  if( del_list().size() > 0 )
    {
      os << (add_list().size() > 0 ? " (del " : "(del ");
      del_list().print( os );
      os << ")";
    }
}

bool
stripsEffect_t::operator==( const stripsEffect_t &effect ) const
{
  return( (add_list() == effect.add_list()) &&
	  (del_list() == effect.del_list()) );
}

stripsEffect_t&
stripsEffect_t::operator=( const stripsEffect_t &effect )
{
  add_list() = effect.add_list();
  del_list() = effect.del_list();
  return( *this );
}


/*******************************************************************************
 *
 * conditional effect
 *
 ******************************************************************************/

bool
conditionalEffect_t::affect( state_t &resulting_state,
                             state_t &original_state,
                             bool nprec ) const
{
  return( precondition().holds( original_state, nprec ) &&
          s_effect().affect( resulting_state, original_state, nprec ) );
}

void
conditionalEffect_t::collect_prec_atoms( atomList_t &atoms ) const
{
  for( size_t i = 0; i < precondition().size(); ++i )
    atoms.insert( precondition().atom_list( i ) );
  s_effect().collect_prec_atoms( atoms );
}

void
conditionalEffect_t::collect_add_atoms( atomList_t &atoms ) const
{
  s_effect().collect_add_atoms( atoms );
}

void
conditionalEffect_t::collect_del_atoms( atomList_t &atoms ) const
{
  s_effect().collect_del_atoms( atoms );
}

void
conditionalEffect_t::print( std::ostream &os ) const
{
  os << "(when (";
  precondition().print( os );
  os << ") ";
  s_effect().print( os );
  os << ")";
}

bool
conditionalEffect_t::operator==( const conditionalEffect_t &effect ) const
{
  return( (precondition() == effect.precondition()) &&
	  (s_effect() == effect.s_effect()) );
}

conditionalEffect_t&
conditionalEffect_t::operator=( const conditionalEffect_t &effect )
{
  precondition() = effect.precondition();
  s_effect() = effect.s_effect();
  return( *this );
}


/*******************************************************************************
 *
 * deterministic effect
 *
 ******************************************************************************/

deterministicEffect_t::~deterministicEffect_t()
{
  for( size_t i = 0; i < c_effect().size(); ++i )
    delete &c_effect().effect( i );
}

bool
deterministicEffect_t::empty( void ) const
{
  if( !s_effect().empty() ) return( false );
  for( size_t i = 0; i < c_effect().size(); ++i )
    if( !c_effect().effect( i ).empty() )
      return( false );
  return( true );
}

void
deterministicEffect_t::insert_effect( const stripsEffect_t &seff )
{
  for( size_t i = 0; i < seff.add_list().size(); ++i )
    s_effect().add_list().insert( seff.add_list().atom( i ) );
  for( size_t i = 0; i < seff.del_list().size(); ++i )
    s_effect().del_list().insert( seff.del_list().atom( i ) );
}

bool
deterministicEffect_t::affect( state_t &resulting_state,
                               state_t &original_state,
                               bool nprec ) const
{
  bool rv = false;
  rv = s_effect().affect( resulting_state, original_state, nprec ) || rv;
  rv = c_effect().affect( resulting_state, original_state, nprec ) || rv;
  return( rv );
}

void
deterministicEffect_t::collect_prec_atoms( atomList_t &atoms ) const
{
  s_effect().collect_prec_atoms( atoms );
  for( size_t i = 0; i < c_effect().size(); ++i )
    c_effect().effect( i ).collect_prec_atoms( atoms );
}

void
deterministicEffect_t::collect_add_atoms( atomList_t &atoms ) const
{
  s_effect().collect_add_atoms( atoms );
  for( size_t i = 0; i < c_effect().size(); ++i )
    c_effect().effect( i ).collect_add_atoms( atoms );
}

void
deterministicEffect_t::collect_del_atoms( atomList_t &atoms ) const
{
  s_effect().collect_del_atoms( atoms );
  for( size_t i = 0; i < c_effect().size(); ++i )
    c_effect().effect( i ).collect_del_atoms( atoms );
}

void
deterministicEffect_t::print( std::ostream &os ) const
{
  s_effect().print( os );
  if( c_effect().size() > 0 )
    {
      if( (s_effect().add_list().size() > 0) ||
	  (s_effect().del_list().size() > 0) )
	os << " ";
      c_effect().print( os );
    }
}

bool
deterministicEffect_t::operator==( const deterministicEffect_t &effect ) const
{
  return( (s_effect() == effect.s_effect()) &&
	  (c_effect() == effect.c_effect()) );
}

deterministicEffect_t&
deterministicEffect_t::operator=( const deterministicEffect_t &effect )
{
  s_effect() = effect.s_effect();
  c_effect() = effect.c_effect();
  return( *this );
}


/*******************************************************************************
 *
 * probabilistic effect
 *
 ******************************************************************************/

bool
probabilisticEffect_t::affect( state_t &resulting_state,
                               state_t &original_state,
                               bool nprec ) const
{
  return( deterministicEffect_t::affect( resulting_state,
                                         original_state,
                                         nprec ) );
}

void
probabilisticEffect_t::print( std::ostream &os ) const
{
  os << "(" << probability() << " ";
  deterministicEffect_t::print( os );
  os << ")";
}

bool
probabilisticEffect_t::operator==( const probabilisticEffect_t &effect ) const
{
  return( (s_effect() == effect.s_effect()) &&
	  (c_effect() == effect.c_effect()) );
}

probabilisticEffect_t&
probabilisticEffect_t::operator=( const probabilisticEffect_t &effect )
{
  probability_ = effect.probability();
  s_effect() = effect.s_effect();
  c_effect() = effect.c_effect();
  return( *this );
}


/*******************************************************************************
 *
 * action
 *
 ******************************************************************************/

action_t::action_t( const std::string &name, const std::string &nameXML )
  : ref_count_(0)
{
  action_t::register_use( this );
  name_ = strdup( name.c_str() );
  nameXML_ = strdup( nameXML.c_str() );
}

action_t::~action_t()
{
  assert( ref_count_ == 0 );
  free( (void*)name_ );
  free( (void*)nameXML_ );
}

void
action_t::insert_precondition( const atomList_t &alist )
{
  atomList_t *al = new atomList_t;
  *al = alist;
  precondition().insert( al );
}

void
action_t::insert_precondition( const atomListList_t &alist )
{
  for( size_t i = 0; i < alist.size(); ++i )
    insert_precondition( alist.atom_list( i ) );
}


/*******************************************************************************
 *
 * deterministic action
 *
 ******************************************************************************/

deterministicAction_t::deterministicAction_t( const std::string &name, const std::string &nameXML )
    : action_t(name,nameXML)
{
  notify( this, "deterministicAction_t::deterministicAction_t(std::string&,std::string&)" );
}

deterministicAction_t::~deterministicAction_t()
{
}

bool
deterministicAction_t::affect( state_t& state, bool nprec ) const
{
  state_t *original_state = new state_t(state);
  bool rv = effect().affect( state, *original_state, nprec );
  state.make_digest();
  delete original_state;
  return( rv );
}

const stateProbList_t&
deterministicAction_t::expand( const state_t &state, bool nprec ) const
{
  state_t *nstate = new state_t( state );
  const stateProbList_t *result = new stateProbList_t( nstate, 1 );
  affect( *nstate, nprec );
  return( *result );
}

void
deterministicAction_t::expand( const state_t &state,
			       std::pair<state_t*,Rational> *list, bool nprec ) const
{
  *list[0].first = state;
  affect( *list[0].first, nprec );
  list[0].second = Rational( 1 );
  list[1].second = Rational( -1 );
}

void
deterministicAction_t::print_full( std::ostream &os ) const
{
  os << "(action " << name() << std::endl
     << "        (prec ";
  precondition().print( os );
  os << ")" << std::endl
     << "        (eff ";
  effect().print( os );
  os << "))";
}

action_t*
deterministicAction_t::clone( void ) const
{
  deterministicAction_t *result = new deterministicAction_t( name(), nameXML() );
  result->precondition() = precondition();
  result->effect() = effect();
  return( result );
}

void
deterministicAction_t::collect_prec_atoms( atomList_t &atoms ) const
{
  for( size_t i = 0; i < precondition().size(); ++i )
    atoms.insert( precondition().atom_list( i ) );
  effect().collect_prec_atoms( atoms );
}

void
deterministicAction_t::collect_add_atoms( atomList_t &atoms ) const
{
  effect().collect_add_atoms( atoms );
}

void
deterministicAction_t::collect_del_atoms( atomList_t &atoms ) const
{
  effect().collect_del_atoms( atoms );
}


/*******************************************************************************
 *
 * probabilistic action
 *
 ******************************************************************************/

probabilisticAction_t::probabilisticAction_t( const std::string &name, const std::string &nameXML )
    : action_t(name,nameXML)
{
  notify( this, "probabilisticAction_t::probabilisticAction_t(std::string&,std::string&)" );
}

probabilisticAction_t::~probabilisticAction_t()
{
  for( size_t i = 0; i < size(); ++i )
    delete &effect( i );
}

bool
probabilisticAction_t::empty( void ) const
{
  for( size_t i = 0; i < size(); ++i )
    if( !effect( i ).empty() )
      return( false );
  return( true );
}

bool
probabilisticAction_t::affect( state_t& state, bool nprec ) const
{
  state_t *original_state = new state_t(state);
  bool rv = effect().affect( state, *original_state, nprec );
  state.make_digest();
  delete original_state;
  return( rv );
}

const stateProbList_t&
probabilisticAction_t::expand( const state_t &state, bool nprec ) const
{
  stateProbList_t *result = new stateProbList_t;
  for( size_t i = 0; i < effect().size(); ++i )
    {
      state_t *nstate = new state_t( state );
      state_t *original_state = new state_t(state);
      effect( i ).affect( *nstate, *original_state, nprec );
      nstate->make_digest();
      delete original_state;
      result->push_back( std::make_pair( nstate, probability( i ) ) );
    }
  return( *result );
}

void
probabilisticAction_t::expand( const state_t &state,
			       std::pair<state_t*,Rational> *state_prob_list, bool nprec ) const
{
  for( size_t i = 0; i < effect().size(); ++i )
    {
      *state_prob_list[i].first = state;
      state_t *original_state = new state_t(state);
      effect( i ).affect( *state_prob_list[i].first, *original_state, nprec );
      delete original_state;
      state_prob_list[i].first->make_digest();
      state_prob_list[i].second = probability( i );
    }
  state_prob_list[effect().size()].second = Rational( -1 );
}

void
probabilisticAction_t::print_full( std::ostream &os ) const
{
  os << "(action " << name() << std::endl
     << "        (prec ";
  precondition().print( os );
  os << ")" << std::endl
     << "        (eff ";
  effect().print( os );
  os << "))";
}

action_t*
probabilisticAction_t::clone( void ) const
{
  probabilisticAction_t *result = new probabilisticAction_t( name(), nameXML() );
  result->precondition() = precondition();
  result->effect() = effect();
  return( result );
}

void
probabilisticAction_t::collect_prec_atoms( atomList_t &atoms ) const
{
  for( size_t i = 0; i < precondition().size(); ++i )
    atoms.insert( precondition().atom_list( i ) );

  for( size_t i = 0; i < effect().size(); ++i )
    effect( i ).collect_prec_atoms( atoms );
}

void
probabilisticAction_t::collect_add_atoms( atomList_t &atoms ) const
{
  for( size_t i = 0; i < effect().size(); ++i )
    effect( i ).collect_add_atoms( atoms );
}

void
probabilisticAction_t::collect_del_atoms( atomList_t &atoms ) const
{
  for( size_t i = 0; i < effect().size(); ++i )
    effect( i ).collect_del_atoms( atoms );
}
