#include "global.h"
#include "actions.h"
#include "effects.h"
#include "problems.h"
#include "formulas.h"
#include "expressions.h"
#include "exceptions.h"
#include "states.h"
#include <assert.h>
#include <stack>

bool ConditionalEffect::working_ = false;
bool ProbabilisticEffect::working_ = false;


/*******************************************************************************
 *
 * assignment
 *
 ******************************************************************************/

Assignment::Assignment( unsigned oper, const Application& application,
			const Expression& expr )
  : operator_(oper), application_(&application), expr_(&expr)
{
  Expression::register_use( application_ );
  Expression::register_use( expr_ );
  notify( this, "Assignment::Assignment(unsigned,Application&,Expression&)" );
}

Assignment::~Assignment()
{
  Expression::unregister_use( application_ );
  Expression::unregister_use( expr_ );
}

void
Assignment::affect( ValueMap& values ) const
{
  if( operator_ == ASSIGN_OP )
    {
      values[application_] = expr_->value( values );
    }
  else
    {
      ValueMap::const_iterator vi = values.find( application_ );
      if( vi == values.end() )
	throw Exception( "changing undefined value" );
      else if( operator_ == SCALE_UP_OP )
	values[application_] = (*vi).second * expr_->value( values );
      else if( operator_ == SCALE_DOWN_OP )
	values[application_] = (*vi).second / expr_->value( values );
      else if( operator_ == INCREASE_OP )
	values[application_] = (*vi).second + expr_->value( values );
      else
	values[application_] = (*vi).second - expr_->value( values );
    }
}

void
Assignment::affect( state_t& state ) const
{
#ifndef NO_STRICT
  throw Exception( "Assignment::affect: error: unsupported effect" );
#else
  std::cout << "Assignment::affect: error: unsupported effect" << std::endl;
#endif
}

const Assignment&
Assignment::instantiation( const SubstitutionMap& subst,
			   const problem_t& problem ) const
{
  return( *new Assignment( operator_, 
			   application_->substitution( subst ),
			   expr_->instantiation( subst, problem ) ) );
}

bool
Assignment::operator==( const Assignment& assig ) const
{
  return( (operator_ == assig.operator_) &&
	  (*application_ == *assig.application_) &&
	  (*expr_ == *assig.expr_) );
}

void
Assignment::print( std::ostream& os, const FunctionTable& functions,
		   const TermTable& terms ) const
{
  os << '(';
  if( operator_ == ASSIGN_OP )
    os << "assign ";
  else if( operator_ == SCALE_UP_OP )
    os << "scale-up ";
  else if( operator_ == SCALE_DOWN_OP )
    os << "scale-down ";
  else if( operator_ == INCREASE_OP )
    os << "increase ";
  else
    os << "decrease ";
  application_->print( os, functions, terms );
  os << ' ';
  expr_->print( os, functions, terms );
  os << ')';
}


/*******************************************************************************
 *
 * add effect
 *
 ******************************************************************************/

AddEffect::AddEffect( const Atom& atom )
  : atom_(&atom)
{
  Effect::register_use( this );
  StateFormula::register_use( atom_ );
  notify( this, "AddEffect::AddEffect(Atom&)" );
}

AddEffect::~AddEffect()
{
  StateFormula::unregister_use( atom_ );
}

const Effect&
AddEffect::flatten( void ) const
{
  Effect::register_use( this );
  return( *this );
}

void
AddEffect::state_change( AtomList& adds, AtomList& deletes, 
			 AssignmentList& assignments,
			 const state_t& state ) const
{
  adds.push_back( atom_ );
}

const stateProbList_t& 
AddEffect::expand( const stateProbList_t &state_list ) const
{
  stateProbList_t *result = new stateProbList_t;
  for( stateProbList_t::const_iterator si = state_list.begin(); si != state_list.end(); ++si )
    {
      state_t *state = new state_t( *((*si).first) );
      state->add( *atom_ );
      state->make_digest();
      assert( state->make_check() );
      result->push_back( std::make_pair( state, (*si).second ) );
    }
  return( *result );
}

void
AddEffect::translate( stripsEffect_t &s_effect,
		      conditionalEffectList_t &c_effect ) const
{
  ushort_t atm = problem_t::atom_hash_get( atom() );
  s_effect.insert_add( atm );
}

const Effect& 
AddEffect::instantiation( const SubstitutionMap& subst,
			  const problem_t& problem ) const
{
  const Atom* inst_atom = &atom().substitution( subst );
  if( inst_atom == atom_ )
    {
      StateFormula::unregister_use( inst_atom );
      Effect::register_use( this );
      return( *this );
    }
  else
    {
      const Effect *result = new AddEffect( *inst_atom );
      StateFormula::unregister_use( inst_atom );
      return( *result );
    }
}

bool
AddEffect::operator==( const Effect& eff ) const
{
  const AddEffect *aeff = dynamic_cast<const AddEffect*>(&eff);
  return( (aeff != NULL) && (atom() == aeff->atom()) );
}

void
AddEffect::print( std::ostream& os, const PredicateTable& predicates,
		  const FunctionTable& functions,
		  const TermTable& terms ) const
{
  atom().print( os, predicates, functions, terms );
}

void
AddEffect::analyze( PredicateTable &predicates, TermTable &terms,
		    std::map<const StateFormula*,const Atom*> &hash ) const
{
}

const Effect&
AddEffect::rewrite( std::map<const StateFormula*,const Atom*> &hash ) const
{
  Effect::register_use( this );
  return( *this );
}


/*******************************************************************************
 *
 * delete effect
 *
 ******************************************************************************/

DeleteEffect::DeleteEffect( const Atom& atom )
  : atom_(&atom)
{
  Effect::register_use( this );
  StateFormula::register_use( atom_ );
  notify( this, "DeleteEffect::DeleteEffect(Atom&)" );
}

DeleteEffect::~DeleteEffect()
{
  StateFormula::unregister_use( atom_ );
}

const Effect&
DeleteEffect::flatten( void ) const
{
  Effect::register_use( this );
  return( *this );
}

void
DeleteEffect::state_change( AtomList& adds, AtomList& deletes,
			    AssignmentList& assignments,
			    const state_t& state ) const
{
  deletes.push_back( atom_ );
}

const stateProbList_t& 
DeleteEffect::expand( const stateProbList_t &state_list ) const
{
  stateProbList_t *result = new stateProbList_t;
  for( stateProbList_t::const_iterator si = state_list.begin(); si != state_list.end(); ++si )
    {
      state_t *state = new state_t( *(*si).first );
      state->clear( *atom_ );
      state->make_digest();
      assert( state->make_check() );
      result->push_back( std::make_pair( state, (*si).second ) );
    }
  return( *result );
}

void
DeleteEffect::translate( stripsEffect_t &s_effect,
			 conditionalEffectList_t &c_effect ) const
{
  ushort_t atm = problem_t::atom_hash_get( atom() );
  s_effect.insert_del( atm );
}

const Effect&
DeleteEffect::instantiation( const SubstitutionMap& subst,
			     const problem_t& problem ) const
{
  const Atom* inst_atom = &atom().substitution( subst );
  if( inst_atom == atom_ )
    {
      StateFormula::unregister_use( inst_atom );
      Effect::register_use( this );
      return( *this );
    }
  else
    {
      const Effect *result = new DeleteEffect( *inst_atom );
      StateFormula::unregister_use( inst_atom );
      return( *result );
    }
}

bool
DeleteEffect::operator==( const Effect& eff ) const
{
  const DeleteEffect *deff = dynamic_cast<const DeleteEffect*>(&eff);
  return( (deff != NULL) && (atom() == deff->atom()) );
}

void
DeleteEffect::print( std::ostream& os, const PredicateTable& predicates,
		     const FunctionTable& functions,
		     const TermTable& terms ) const
{
  os << "(not ";
  atom().print( os, predicates, functions, terms );
  os << ")";
}

void
DeleteEffect::analyze( PredicateTable &predicates, TermTable &terms,
		       std::map<const StateFormula*,const Atom*> &hash ) const
{
}

const Effect&
DeleteEffect::rewrite( std::map<const StateFormula*,const Atom*> &hash ) const
{
  Effect::register_use( this );
  return( *this );
}


/*******************************************************************************
 *
 * assignment effect
 *
 ******************************************************************************/

AssignmentEffect::AssignmentEffect( const Assignment& assignment )
  : assignment_(&assignment)
{
  Effect::register_use( this );
  notify( this, "AssignmentEffect::AssignmentEffect(Assignment&)" );
}

AssignmentEffect::~AssignmentEffect()
{
  delete assignment_;
}

const Effect&
AssignmentEffect::flatten( void ) const
{
#ifndef NO_STRICT
  throw Exception( "AssignmentEffect::flatten: error: unsupported effect" );
#else
#  if 0
  std::cout << "AssignmentEffect::flatten: error: unsupported effect" << std::endl;
  Effect::register_use( this );
  return( *this );
#  else
  return( *new ConjunctiveEffect() );
#  endif
#endif
}

void
AssignmentEffect::state_change( AtomList& adds, AtomList& deletes,
				AssignmentList& assignments,
				const state_t& state ) const
{
  assignments.push_back( assignment_ );
}

const stateProbList_t& 
AssignmentEffect::expand( const stateProbList_t &state_list ) const
{
#ifndef NO_STRICT
  throw Exception( "Assignment::expand: error: unsupported effect" );
#else
  stateProbList_t *result = new stateProbList_t;
  for( stateProbList_t::const_iterator si = state_list.begin(); si != state_list.end(); ++si )
    {
      state_t *state = new state_t( *((*si).first) );
      state->make_digest();
      assert( state->make_check() );
      result->push_back( std::make_pair( state, (*si).second ) );
    }
  return( *result );
#endif
}

void
AssignmentEffect::translate( stripsEffect_t &s_effect,
			     conditionalEffectList_t &c_effect ) const
{
#ifndef NO_STRICT
  throw Exception( "Assignment::translate: error: unsupported effect" );
#else
  std::cout << "Assignment::translate: error: unsupported effect" << std::endl;
#endif
}

const Effect& 
AssignmentEffect::instantiation( const SubstitutionMap& subst,
				 const problem_t& problem ) const
{
  return( *new AssignmentEffect( assignment().instantiation( subst, problem ) ) );
}

bool
AssignmentEffect::operator==( const Effect& eff ) const
{
  const AssignmentEffect *assig = dynamic_cast<const AssignmentEffect*>(&eff);
  return( (assig != NULL) && (assignment() == assig->assignment()) );
}

void
AssignmentEffect::print( std::ostream& os,
			 const PredicateTable& predicates,
			 const FunctionTable& functions,
			 const TermTable& terms ) const
{
  assignment().print( os, functions, terms );
}

void
AssignmentEffect::analyze( PredicateTable &predicates, TermTable &terms,
			   std::map<const StateFormula*,const Atom*> &hash ) const
{
}

const Effect&
AssignmentEffect::rewrite( std::map<const StateFormula*,const Atom*> &hash ) const
{
  Effect::register_use( this );
  return( *this );
}


/*******************************************************************************
 *
 * conjunctive effect
 *
 ******************************************************************************/

ConjunctiveEffect::ConjunctiveEffect()
{
  Effect::register_use( this );
  notify( this, "ConjunctiveEffect::ConjunctiveEffect()" );
}

ConjunctiveEffect::~ConjunctiveEffect()
{
  for( EffectList::const_iterator ei = conjuncts_.begin(); ei != conjuncts_.end(); ++ei )
    Effect::unregister_use( *ei );
}

void
ConjunctiveEffect::add_conjunct( const Effect& conjunct )
{
  const ConjunctiveEffect* conj_effect = dynamic_cast<const ConjunctiveEffect*>(&conjunct);
  if( conj_effect != NULL )
    {
      for( EffectList::const_iterator ei = conj_effect->conjuncts_.begin();
	   ei != conj_effect->conjuncts_.end(); ++ei )
	{
	  Effect::register_use( *ei );
	  conjuncts_.push_back( *ei );
	}
      Effect::unregister_use( &conjunct );
    }
  else
    {
      conjuncts_.push_back( &conjunct );
    }
}

const Effect&
ConjunctiveEffect::flatten( void ) const
{
  if( size() == 0 )
    {
      Effect::register_use( this );
      return( *this );
    }
  else
    {
      const ProbabilisticEffect *prob_eff = new ProbabilisticEffect;
      const ProbabilisticEffect *tmp;

      for( size_t i = 0; i < size(); ++i )
	{
	  const Effect *eff = &conjunct( i ).flatten();
	  const ProbabilisticEffect *peff = dynamic_cast<const ProbabilisticEffect*>( eff );
	  if( peff == NULL )
	    {
	      ProbabilisticEffect *peff = new ProbabilisticEffect;
	      peff->add_outcome( Rational(1), *eff );
	      tmp = &prob_eff->cross_product( *peff );
	      Effect::unregister_use( peff );
	    }
	  else
	    {
	      tmp = &prob_eff->cross_product( *peff );
	      Effect::unregister_use( eff );
	    }
	  Effect::unregister_use( prob_eff );
	  prob_eff = tmp;
	}
      
      assert( prob_eff->size() > 0 );
      if( prob_eff->size() > 1 )
	return( *prob_eff );
      else
	{
	  const Effect *result = &prob_eff->effect( 0 );
	  Effect::register_use( result );
	  Effect::unregister_use( prob_eff );
	  return( *result );
	}
    }
}

void
ConjunctiveEffect::state_change( AtomList& adds, AtomList& deletes,
				 AssignmentList& assignments,
				 const state_t& state ) const
{
  for( EffectList::const_iterator ei = conjuncts_.begin(); ei != conjuncts_.end(); ++ei )
    (*ei)->state_change( adds, deletes, assignments, state );
}

const stateProbList_t& 
ConjunctiveEffect::expand( const stateProbList_t &state_list ) const
{
  const stateProbList_t *result = new stateProbList_t( state_list );
  for( EffectList::const_iterator ei = conjuncts_.begin(); ei != conjuncts_.end(); ++ei )
    {
      const stateProbList_t *tmp = &(*ei)->expand( *result );
      for( stateProbList_t::const_iterator si = result->begin(); si != result->end(); ++si )
	delete (*si).first;
      delete result;
      result = tmp;
    }
  return( *result );
}

void
ConjunctiveEffect::translate( stripsEffect_t &s_effect,
			      conditionalEffectList_t &c_effect ) const
{
  for( size_t i = 0; i < size(); ++i )
    conjunct( i ).translate( s_effect, c_effect );
}

const Effect& 
ConjunctiveEffect::instantiation( const SubstitutionMap& subst,
				  const problem_t& problem ) const
{
  ConjunctiveEffect& inst_effect = *new ConjunctiveEffect();
  for( EffectList::const_iterator ei = conjuncts_.begin(); ei != conjuncts_.end(); ++ei )
    inst_effect.add_conjunct( (*ei)->instantiation( subst, problem ) );
  return( inst_effect );
}

bool
ConjunctiveEffect::operator==( const Effect& eff ) const
{
  const ConjunctiveEffect *ceff = dynamic_cast<const ConjunctiveEffect*>(&eff);
  if( (ceff != NULL) && (size() == ceff->size()) )
    {
      for( size_t i = 0; i < size(); ++i )
	if( !(conjunct( i ) == ceff->conjunct( i )) )
	  return( false );
      return( true );
    }
  return( true );
}

void
ConjunctiveEffect::print( std::ostream& os,
			  const PredicateTable& predicates,
			  const FunctionTable& functions,
			  const TermTable& terms ) const
{
  if( size() == 1 )
    conjunct(0).print( os, predicates, functions, terms );
  else
    {
      os << "(and";
      for( EffectList::const_iterator ei = conjuncts_.begin();
	   ei != conjuncts_.end(); ++ei )
	{
	  os << ' ';
	  (*ei)->print( os, predicates, functions, terms );
	}
      os << ")";
    }
}

void
ConjunctiveEffect::analyze( PredicateTable &predicates, TermTable &terms,
			    std::map<const StateFormula*,const Atom*> &hash ) const
{
  for( EffectList::const_iterator ei = conjuncts_.begin(); ei != conjuncts_.end(); ++ei )
    (*ei)->analyze( predicates, terms, hash );
}

const Effect&
ConjunctiveEffect::rewrite( std::map<const StateFormula*,const Atom*> &hash ) const
{
  ConjunctiveEffect *conj = new ConjunctiveEffect;
  for( EffectList::const_iterator ei = conjuncts_.begin(); ei != conjuncts_.end(); ++ei )
    conj->add_conjunct( (*ei)->rewrite( hash ) );
  return( *conj );
}


/*******************************************************************************
 *
 * conditional effect
 *
 ******************************************************************************/

const Effect&
ConditionalEffect::make( const StateFormula& condition, const Effect& effect )
{
  if( condition.tautology() )
    {
      StateFormula::unregister_use( &condition );
      return( effect );
    }
  else if( condition.contradiction() )
    {
      StateFormula::unregister_use( &condition );
      Effect::unregister_use( &effect );
      return( *new ConjunctiveEffect() );
    }
  else
    {
      const Effect *eff = new ConditionalEffect( condition, effect );
      StateFormula::unregister_use( &condition );
      Effect::unregister_use( &effect );
      return( *eff );
    }
}

ConditionalEffect::ConditionalEffect( const StateFormula& condition,
				      const Effect& effect )
  : condition_(&condition), effect_(&effect)
{
  Effect::register_use( this );
  StateFormula::register_use( condition_ );
  Effect::register_use( effect_ );
  notify( this, "ConditionalEffect::ConditionalEffect(StateFormula&,Effect&)" );
}

ConditionalEffect::~ConditionalEffect()
{
  StateFormula::unregister_use( condition_ );
  Effect::unregister_use( effect_ );
}

const Effect&
ConditionalEffect::flatten( void ) const
{
  const Effect *effect = &effect_->flatten();
  const ProbabilisticEffect *prob_effect = dynamic_cast<const ProbabilisticEffect*>(effect);
  const ConditionalEffect *cond_effect = dynamic_cast<const ConditionalEffect*>(effect);

  if( effect == this )
    {
      Effect::unregister_use( effect );
      Effect::register_use( this );
      return( *this );
    }
  else if( prob_effect != NULL )
    {
      ProbabilisticEffect *result = new ProbabilisticEffect;
      for( size_t i = 0; i < prob_effect->size(); ++i )
	{
	  StateFormula::register_use( &condition() );
	  Effect::register_use( &prob_effect->effect( i ) );
	  const Effect *ceff = &make( condition(), prob_effect->effect( i ) );
	  result->add_outcome( prob_effect->probability(i), ceff->flatten() );
	  Effect::unregister_use( ceff );
	}
      Effect::unregister_use( effect );
      return( *result );
    }
  else if( cond_effect != NULL )
    {
      Conjunction *cond = new Conjunction;
      StateFormula::register_use( &condition() );
      cond->add_conjunct( condition() );
      StateFormula::register_use( &cond_effect->condition() );
      cond->add_conjunct( cond_effect->condition() );

      Effect::register_use( &cond_effect->effect() );
      const Effect *result = &make( *cond, cond_effect->effect() );
      Effect::unregister_use( effect );
      return( *result );
    }
  else
    {
      StateFormula::register_use( &condition() );
      const Effect *result = &make( condition(), *effect );
      return( *result );
    }
}

void
ConditionalEffect::state_change( AtomList& adds, AtomList& deletes,
				 AssignmentList& assignments,
				 const state_t& state ) const
{
  if( condition().holds( state ) )
    effect().state_change( adds, deletes, assignments, state );
}

const stateProbList_t& 
ConditionalEffect::expand( const stateProbList_t &state_list ) const
{
  stateProbList_t simple_list;
  stateProbList_t *result = new stateProbList_t;
  for( stateProbList_t::const_iterator si = state_list.begin(); si != state_list.end(); ++si )
    if( !condition().holds( *(*si).first ) )
      {
	state_t *state = new state_t( *(*si).first );
	result->push_back( std::make_pair( state, (*si).second ) );
      }
    else
      {
	simple_list.push_back( (*si) );
	const stateProbList_t *tmp = &effect().expand( simple_list );
	result->insert( result->end(), tmp->begin(), tmp->end() );
	delete tmp;
	simple_list.pop_back();
      }
  return( *result );
}

void
ConditionalEffect::translate( stripsEffect_t &s_effect,
			      conditionalEffectList_t &c_effect ) const
{
  if( working_ )
    throw Exception( "ConditionalEffect::translate: error: already working" );

  working_ = true;
  conditionalEffect_t *ceffect = new conditionalEffect_t;

  // translate condition
  condition().translate( ceffect->precondition() );

  // translate effect
  effect().translate( ceffect->s_effect(), c_effect );

  // insert into list
  c_effect.insert( ceffect );
  working_ = false;
}

const Effect&
ConditionalEffect::instantiation( const SubstitutionMap& subst,
				  const problem_t& problem ) const
{
  const StateFormula *cond = &condition().instantiation( subst, problem );
  const Effect *eff = &effect().instantiation( subst, problem );
  const Effect *result = &make( *cond, *eff );
  return( *result );
}

bool
ConditionalEffect::operator==( const Effect& eff ) const
{
  const ConditionalEffect *ceff = dynamic_cast<const ConditionalEffect*>(&eff);
  return( (ceff != NULL) && (condition() == ceff->condition()) && (effect() == ceff->effect()) );
}

void
ConditionalEffect::print( std::ostream& os,
			  const PredicateTable& predicates,
			  const FunctionTable& functions,
			  const TermTable& terms ) const
{
  os << "(when ";
  condition().print( os, predicates, functions, terms );
  os << ' ';
  effect().print( os, predicates, functions, terms );
  os << ")";
}

void
ConditionalEffect::analyze( PredicateTable &predicates, TermTable &terms,
			    std::map<const StateFormula*,const Atom*> &hash ) const
{
  condition().analyze( predicates, terms, hash );
  effect().analyze( predicates, terms, hash );
}

const Effect&
ConditionalEffect::rewrite( std::map<const StateFormula*,const Atom*> &hash ) const
{
  return( ConditionalEffect::make( condition().rewrite( hash ), effect().rewrite( hash ) ) );
}


/*******************************************************************************
 *
 * probabilistic effect
 *
 ******************************************************************************/

ProbabilisticEffect::ProbabilisticEffect()
  : weight_sum_(0)
{
  Effect::register_use( this );
  notify( this, "ProbabilisticEffect::ProbabilisticEffect()" );
}

ProbabilisticEffect::~ProbabilisticEffect()
{
  for( EffectList::const_iterator ei = effects_.begin(); ei != effects_.end(); ++ei )
    Effect::unregister_use( *ei );
}

const ProbabilisticEffect&
ProbabilisticEffect::cross_product( const ProbabilisticEffect &prob_effect ) const
{
  ProbabilisticEffect *result = new ProbabilisticEffect;

  Rational sum = 0, sum1 = 0, sum2 = 0;
  for( size_t i = 0; i < size(); ++i )
    {
      for( size_t j = 0; j < prob_effect.size(); ++j )
	{
	  ConjunctiveEffect *conjunction = new ConjunctiveEffect;
	  Effect::register_use( &effect( i ) );
	  conjunction->add_conjunct( effect( i ) );
	  Effect::register_use( &prob_effect.effect( j ) );
	  conjunction->add_conjunct( prob_effect.effect( j ) );
	  result->add_outcome( probability(i) * prob_effect.probability(j), *conjunction );
	  if( sum1 == 0 ) sum2 = sum2 + prob_effect.probability( j );
	  sum = sum + probability(i) * prob_effect.probability(j);
	}
      sum1 = sum1 + probability( i );
    }
  
  if( sum1 != 1 )
    {
      for( size_t j = 0; j < prob_effect.size(); ++j )
	{
	  Effect::register_use( &prob_effect.effect( j ) );
	  result->add_outcome( (1 - sum1) * prob_effect.probability( j ),
			       prob_effect.effect( j ) );
	}
    }

  if( sum2 != 1 )
    {
      for( size_t i = 0; i < size(); ++i )
	{
	  Effect::register_use( &effect( i ) );
	  result->add_outcome( (1 - sum2) * probability( i ), effect( i ) );
	}
    }

  return( *result );
}

bool
ProbabilisticEffect::add_outcome( const Rational& p, const Effect& effect)
{
  const ProbabilisticEffect* prob_effect =
    dynamic_cast<const ProbabilisticEffect*>(&effect);
  if( prob_effect != NULL )
    {
      for( size_t i = 0; i < prob_effect->size(); ++i )
	{
	  Effect::register_use( &prob_effect->effect( i ) );
	  if( !add_outcome( p*prob_effect->probability(i), prob_effect->effect(i) ) )
	    return( false );
	}
      Effect::unregister_use( &effect );
    }
  else if( p != 0 )
    {
      effects_.push_back( &effect );
      if( weight_sum_ == 0 )
	{
	  weights_.push_back( p.numerator() );
	  weight_sum_ = p.denominator();
	  return( true );
	}
      else
	{
	  std::pair<int,int> m = Rational::multipliers( weight_sum_, p.denominator() );
	  int sum = 0;
	  for( size_t i = 0; i < size(); ++i )
	    {
	      weights_[i] *= m.first;
	      sum += weights_[i];
	    }
	  weights_.push_back( p.numerator() * m.second );
	  sum += p.numerator() * m.second;
	  weight_sum_ *= m.first;
	  return( sum <= weight_sum_ );
	}
    }
  return( true );
}

Rational
ProbabilisticEffect::probability( size_t i ) const
{
  return Rational( weights_[i], weight_sum_ );
}

const Effect&
ProbabilisticEffect::flatten( void ) const
{
  if( size() == 0 )
    {
      Effect::register_use( this );
      return( *this );
    }
  else
    {
      ProbabilisticEffect *result = new ProbabilisticEffect;
      for( size_t i = 0; i < size(); ++i )
	{
	  const Effect *eff = &effect( i ).flatten();
	  const ProbabilisticEffect *peff = dynamic_cast<const ProbabilisticEffect*>( eff );
	  if( peff != NULL )
	    {
	      for( size_t j = 0; j < peff->size(); ++j )
		{
		  Effect::register_use( &peff->effect( j ) );
		  result->add_outcome( probability(i)*peff->probability(j), peff->effect(j) );
		}
	      Effect::unregister_use( peff );
	    }
	  else
	    {
	      result->add_outcome( probability( i ), *eff );
	    }
	}

      if( (result->size() != 1) || (result->probability( 0 ) != 1) )
	return( *result );
      else
	{
	  const Effect *tmp = &result->effect( 0 );
	  Effect::register_use( tmp );
	  Effect::unregister_use( result );
	  return( *tmp );
	}
    }
}

void
ProbabilisticEffect::state_change( AtomList& adds, AtomList& deletes,
				   AssignmentList& assignments,
				   const state_t& state ) const
{
  if( size() != 0 )
    {
      int w = int(rand()/(RAND_MAX + 1.0)*weight_sum_);
      int wtot = 0;
      size_t n = size();
      for( size_t i = 0; i < n; ++i )
	{
	  wtot += weights_[i];
	  if( w < wtot )
	    {
	      effect( i ).state_change( adds, deletes, assignments, state );
	      return;
	    }
	}
    }
}

const stateProbList_t& 
ProbabilisticEffect::expand( const stateProbList_t &state_list ) const
{
  Rational sum( 0 );
  stateProbList_t *tmp = new stateProbList_t;
  for( size_t i = 0; i < size(); ++i )
    {
      Rational p( weights_[i], weight_sum_ );
      sum = sum + p;
      const stateProbList_t *effr = &effect( i ).expand( state_list );
      for( stateProbList_t::const_iterator si = effr->begin(); si != effr->end(); ++si )
	tmp->push_back( std::make_pair( (*si).first, (*si).second * p ) );
      delete effr;
    }

  // remaining effect
  if( sum != 1 )
    {
      Rational r = 1 - sum;
      for( stateProbList_t::const_iterator si = state_list.begin(); si != state_list.end(); ++si )
	{
	  state_t *state = new state_t( *(*si).first );
	  tmp->push_back( std::make_pair( state, (*si).second * r ) );
	}
    }

  // consolidate results
  stateProbList_t *result = new stateProbList_t;
  for( stateProbList_t::iterator i = tmp->begin(); i != tmp->end(); ++i )
    {
      stateProbList_t::iterator j;
      for( j = result->begin(); j != result->end(); ++j )
	if( *(*j).first == *(*i).first )
	  {
	    (*j).second = (*j).second + (*i).second;
	    delete (*i).first;
	    break;
	  }
      if( j == result->end() ) result->push_back( (*i) );
    }
  delete tmp;
  return( *result );
}

void
ProbabilisticEffect::translate( probabilisticEffectList_t &plist ) const
{
  if( working_ )
    throw Exception( "ProbabilisticEffect::translate: error: already working" );

  working_ = true;
  Rational sum = 0;
  for( size_t i = 0; i < size(); ++i )
    {
      probabilisticEffect_t *peffect = new probabilisticEffect_t( probability( i ) );
      effect( i ).translate( peffect->s_effect(), peffect->c_effect() );
      if( !plist.insert( peffect ) ) delete peffect;
      sum = sum + probability( i );
    }

  // null effect
  if( sum != 1 )
    {
      probabilisticEffect_t *peffect = new probabilisticEffect_t( 1 - sum );
      if( !plist.insert( peffect ) ) delete peffect;
    }
  working_ = false;
}

void
ProbabilisticEffect::translate( stripsEffect_t &s_effect,
				conditionalEffectList_t &c_effect ) const
{
  throw Exception( "ProbabilisticEffect::translate: erroneously called" );
}

const Effect&
ProbabilisticEffect::instantiation( const SubstitutionMap& subst, const problem_t& problem ) const
{
  ProbabilisticEffect& inst_effect = *new ProbabilisticEffect();
  for( size_t i = 0; i < size(); ++i )
    inst_effect.add_outcome( probability( i ), effect( i ).instantiation( subst, problem ) );
  return( inst_effect );
}

bool
ProbabilisticEffect::operator==( const Effect& eff ) const
{
  const ProbabilisticEffect *peff = dynamic_cast<const ProbabilisticEffect*>(&eff);
  if( (peff != NULL) && (size() == peff->size()) )
    {
      for( size_t i = 0; i < size(); ++i )
	if( !(effect( i ) == peff->effect( i )) )
	  return( false );
      return( true );
    }
  return( false );
}

void
ProbabilisticEffect::print( std::ostream& os,
			    const PredicateTable& predicates,
			    const FunctionTable& functions,
			    const TermTable& terms ) const
{
  if( weight_sum_ == 0 )
    os << "(and)";
  else if( weight_sum_ == weights_.back() )
    {
      os << "(probabilistic 1 ";
      effect(0).print( os, predicates, functions, terms );
      os << ")";
    }
  else
    {
      os << "(probabilistic";
      size_t n = size();
      for( size_t i = 0; i < n; ++i )
	{
	  os << ' ' << probability( i ) << ' ';
	  effect( i ).print( os, predicates, functions, terms );
	}
      os << ")";
    }
}

void
ProbabilisticEffect::analyze( PredicateTable &predicates, TermTable &terms,
			      std::map<const StateFormula*,const Atom*> &hash ) const
{
  for( size_t i = 0; i < size(); ++i )
    effect( i ).analyze( predicates, terms, hash );
}

const Effect&
ProbabilisticEffect::rewrite( std::map<const StateFormula*,const Atom*> &hash ) const
{
  ProbabilisticEffect *prob = new ProbabilisticEffect;
  for( size_t i = 0; i < size(); ++i )
    prob->add_outcome( probability( i ), effect( i ).rewrite( hash ) );
  return( *prob );
}


/*******************************************************************************
 *
 * quantified effect
 *
 ******************************************************************************/

QuantifiedEffect::QuantifiedEffect( const Effect& effect )
  : effect_(&effect)
{
  Effect::register_use( this );
  Effect::register_use( effect_ );
  notify( this, "QuantifiedEffect::QuantifiedEffect(Effect&)" );
}

QuantifiedEffect::~QuantifiedEffect()
{
  Effect::unregister_use( effect_ );
}

const Effect&
QuantifiedEffect::flatten( void ) const
{
#ifndef NO_STRICT
  throw Exception( "QuantifiedEffect::flatten erroneously called" );
#else
  std::cout << "QuantifiedEffect::flatten erroneously called" << std::endl;
  Effect::register_use( this );
  return( *this );
#endif
}

void
QuantifiedEffect::state_change( AtomList& adds, AtomList& deletes,
				AssignmentList& assignments,
				const state_t& state ) const
{
  effect().state_change( adds, deletes, assignments, state );
}

const stateProbList_t& 
QuantifiedEffect::expand( const stateProbList_t &state_list ) const
{
  return( effect().expand( state_list ) );
}

void
QuantifiedEffect::translate( stripsEffect_t &s_effect,
			     conditionalEffectList_t &c_effect ) const
{
#ifndef NO_STRICT
  throw Exception( "QuantifiedEffect::translate: erroneously called" );
#else
  std::cout << "QuantifiedEffect::translate: erroneously called" << std::endl;
#endif
}

const Effect&
QuantifiedEffect::instantiation( const SubstitutionMap& subst,
				 const problem_t& problem ) const
{
  int n = arity();
  if( n == 0 )
    return( effect().instantiation( subst, problem ) );
  else
    {
      SubstitutionMap args( subst );
      std::vector<ObjectList> arguments( n, ObjectList() );
      std::vector<ObjectList::const_iterator> next_arg;
      for( int i = 0; i < n; ++i )
	{
	  problem.compatible_objects( arguments[i], problem.terms().type( parameter(i) ) );
	  if( arguments[i].empty() ) return( *new ConjunctiveEffect() );
	  next_arg.push_back( arguments[i].begin() );
	}

      ConjunctiveEffect* conj = new ConjunctiveEffect();
      std::stack<const Effect*> conjuncts;
      conjuncts.push( &effect().instantiation( args, problem ) );
      //Effect::register_use( conjuncts.top() );
      for( int i = 0; i < n; )
	{
	  SubstitutionMap pargs;
	  pargs.insert( std::make_pair( parameter(i), *next_arg[i] ) );
	  const Effect& conjunct = conjuncts.top()->instantiation( pargs, problem );
	  conjuncts.push( &conjunct );
	  if( i + 1 == n )
	    {
	      conj->add_conjunct( conjunct );
	      for( int j = i; j >= 0; --j )
		{
		  if( j < i ) Effect::unregister_use( conjuncts.top() );
		  conjuncts.pop();
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
	    {
	      //Effect::register_use( conjuncts.top() );
	      ++i;
	    }
	}

      while( !conjuncts.empty() )
	{
	  Effect::unregister_use( conjuncts.top() );
	  conjuncts.pop();
	}
      return( *conj );
    }
}

bool
QuantifiedEffect::operator==( const Effect& eff ) const
{
  return( false );
}

void
QuantifiedEffect::print( std::ostream& os,
			 const PredicateTable& predicates,
			 const FunctionTable& functions,
			 const TermTable& terms ) const
{
  if( parameters_.empty() )
    {
      effect().print( os, predicates, functions, terms );
    }
  else
    {
      os << "(forall (";
      VariableList::const_iterator vi = parameters_.begin();
      terms.print_term( os, *vi );
      for( ++vi; vi != parameters_.end(); ++vi )
	{
	  os << ' ';
	  terms.print_term( os, *vi );
	}
      os << ") ";
      effect().print( os, predicates, functions, terms );
      os << ")";
    }
}

void
QuantifiedEffect::analyze( PredicateTable &predicates, TermTable &terms,
			   std::map<const StateFormula*,const Atom*> &hash ) const
{
  effect().analyze( predicates, terms, hash );
}

const Effect&
QuantifiedEffect::rewrite( std::map<const StateFormula*,const Atom*> &hash ) const
{
  QuantifiedEffect *q = new QuantifiedEffect( effect().rewrite( hash ) );
  Effect::unregister_use( &effect() );
  q->parameters_ = parameters_;
  return( *q );
}
