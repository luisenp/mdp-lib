#include "domains.h"
#include "exceptions.h"
#include "expressions.h"
#include "formulas.h"
#include "problems.h"
#include "states.h"
#include "strxml.h"
#include <sstream>
#include <stack>

const StateFormula& StateFormula::TRUE = Constant::TRUE_;
const StateFormula& StateFormula::FALSE = Constant::FALSE_;

const Constant Constant::TRUE_(true);
const Constant Constant::FALSE_(false);

Atom::AtomTable Atom::atoms;

static unsigned internal_predicate_index = 0;


/*******************************************************************************
 *
 * constant
 *
 ******************************************************************************/

Constant::Constant( bool value )
  : value_(value)
{
  StateFormula::register_use(this);
  notify( this, "Constant::Constant(bool)" );
}

bool
Constant::holds( const AtomSet& atoms, const ValueMap& values ) const
{
  return( value_ );
}

bool
Constant::holds( const state_t& state ) const
{
  return( value_ );
}

const StateFormula&
Constant::flatten( bool negated ) const
{
  StateFormula::register_use( this );
  return( *this );
}

void
Constant::translate( atomList_t &alist ) const
{
}

void
Constant::translate( atomListList_t &alist ) const
{
  atomList_t *al = new atomList_t;
  translate( *al );
  alist.insert( al );
}

const Constant&
Constant::instantiation( const SubstitutionMap& subst,
			 const problem_t& problem) const
{
  StateFormula::register_use( this );
  return( *this );
}

bool
Constant::operator==( const StateFormula &formula ) const
{
  const Constant *cons = dynamic_cast<const Constant*>( &formula );
  return( (cons != NULL) && (value_ == cons->value_) );
}

void
Constant::print( std::ostream& os, const PredicateTable& predicates,
		 const FunctionTable& functions, const TermTable& terms ) const
{
  os << (value_?"(and)":"(or)");
}

bool
Constant::match( const StateFormula &formula, VarSubstMap &subst ) const
{
  return( *this == formula );
}

void
Constant::free_vars( VariableList &context, VariableList &vars ) const
{
}

bool
Constant::analyze( PredicateTable &predicates, TermTable &terms,
		   std::map<const StateFormula*,const Atom*> &hash ) const
{
  return( true );
}

const StateFormula&
Constant::rewrite( std::map<const StateFormula*,const Atom*> &hash ) const
{
  StateFormula::register_use( this );
  return( *this );
}

void
Constant::generate_atoms( void ) const
{
}


/*******************************************************************************
 *
 * atom
 *
 ******************************************************************************/

bool
Atom::AtomLess::operator()( const Atom* a1, const Atom* a2 ) const
{
  if( a1->predicate() < a2->predicate() )
    return( true );
  else if( a1->predicate() > a2->predicate() )
    return( false );
  else
    {
      for( size_t i = 0; i < a1->arity(); ++i )
	{
	  if( a1->term( i ) < a2->term( i ) )
	    return( true );
	  else if( a1->term( i ) > a2->term( i ) )
	    return( false );
	}
    }
  return( false );
}

const Atom&
Atom::make_atom( Predicate predicate, const TermList& terms )
{
  Atom *atom = new Atom( predicate );
  bool ground = true;
  for( TermList::const_iterator ti = terms.begin(); ti != terms.end(); ++ti )
    {
      atom->add_term( *ti );
      ground = ground && is_object( *ti );
    }

  if( !ground )
    {
      return( *atom );
    }
  else
    {
      std::pair<AtomTable::const_iterator,bool> result = atoms.insert( atom );
      if( !result.second )
	{
	  StateFormula::unregister_use( atom );
	  StateFormula::register_use( *result.first );
	  return( **result.first );
	}
      else
	{
	  return( *atom );
	}
    }
}

Atom::Atom( Predicate predicate )
  : predicate_(predicate)
{
  StateFormula::register_use( this );
  notify( this, "Atom::Atom(Predicate)" );
}

Atom::~Atom()
{
  AtomTable::const_iterator ai = atoms.find( this );
  if( (ai != atoms.end()) && (*ai == this) ) atoms.erase( ai );
}

bool
Atom::holds( const AtomSet& atoms, const ValueMap& values ) const
{
  return( atoms.find(this) != atoms.end() );
}

bool
Atom::holds( const state_t& state ) const
{
  return( state.holds( *this ) );
}

const Atom&
Atom::substitution( const SubstitutionMap& subst ) const
{
  TermList inst_terms;
  bool substituted = false;
  for( TermList::const_iterator ti = terms_.begin(); ti != terms_.end(); ++ti )
    {
      SubstitutionMap::const_iterator si = (is_variable( *ti )?subst.find( *ti ):subst.end());
      if( si != subst.end() )
	{
	  inst_terms.push_back( (*si).second );
	  substituted = true;
	}
      else
	inst_terms.push_back( *ti );
    }

  if( substituted )
    return( make_atom( predicate(), inst_terms ) );
  else
    {
      StateFormula::register_use( this );
      return( *this );
    }
}

const StateFormula&
Atom::flatten( bool negated ) const
{
  if( !negated )
    {
      StateFormula::register_use( this );
      return( *this );
    }
  else
    {
      return( Negation::make_negation( *this ) );
    }
}

void
Atom::translate( atomList_t &alist ) const
{
  ushort_t atom = problem_t::atom_hash_get( *this );
  alist.insert( atom );
}

void
Atom::translate( atomListList_t &alist ) const
{
  atomList_t *al = new atomList_t;
  translate( *al );
  alist.insert( al );
}

const StateFormula&
Atom::instantiation( const SubstitutionMap& subst,
		     const problem_t& problem ) const
{
  TermList inst_terms;
  bool substituted = false, ground = true;
  size_t objects = 0;

  for( TermList::const_iterator ti = terms_.begin(); ti != terms_.end(); ++ti )
    {
      SubstitutionMap::const_iterator si = (is_variable( *ti )?subst.find( *ti ):subst.end());
      if( si != subst.end() )
	{
	  inst_terms.push_back( (*si).second );
	  substituted = true;
	  ++objects;
	}
      else
	{
	  inst_terms.push_back( *ti );
	  if( is_object(*ti) )
	    ++objects;
	  else
	    ground = false;
	}
    }

  if( substituted || ground )
    {
      const Atom& inst_atom = make_atom( predicate(), inst_terms );
      if( problem.domain().predicates().static_predicate( predicate() ) &&
	  (objects == inst_terms.size()) )
	{
	  if( problem.init_atoms().find( &inst_atom ) != problem.init_atoms().end() )
	    {
	      StateFormula::unregister_use( &inst_atom );
	      StateFormula::register_use( &TRUE );
	      return( TRUE );
	    }
	  else
	    {
	      StateFormula::unregister_use( &inst_atom );
	      StateFormula::register_use( &FALSE );
	      return( FALSE );
	    }
	}
      else
	{
	  return( inst_atom );
	}
    }
  else
    {
      StateFormula::register_use( this );
      return( *this );
    }
}

bool
Atom::operator==( const StateFormula &formula ) const
{
  const Atom *at = dynamic_cast<const Atom*>( &formula );
  if( (at != NULL) && (predicate() == at->predicate()) )
    {
      for( size_t i = 0; i < arity(); ++i )
	if( !(term( i ) == at->term( i )) )
	  return( false );
      return( true );
    }
  return( false );
}

void
Atom::print( std::ostream& os, const PredicateTable& predicates,
	     const FunctionTable& functions, const TermTable& terms ) const {
  os << '(';
  predicates.print_predicate( os, predicate() );
  for( TermList::const_iterator ti = terms_.begin(); ti != terms_.end(); ++ti )
    {
      os << ' ';
      terms.print_term( os, *ti );
    }
  os << ')';
}

void
Atom::printXML( std::ostream& os, const PredicateTable& predicates,
		const FunctionTable& functions, const TermTable& terms) const
{
  os << "<atom><predicate>";
  predicates.print_predicate( os, predicate() );
  os << "</predicate>";
  for( TermList::const_iterator ti = terms_.begin(); ti != terms_.end(); ++ti )
    {
      os << "<term>";
      terms.print_term( os, *ti );
      os << "</term>";
  }
  os << "</atom>";
}

bool
Atom::match( const StateFormula &formula, VarSubstMap &subst ) const
{
  const Atom *at = dynamic_cast<const Atom*>( &formula );
  if( (at != NULL) && (predicate() == at->predicate()) )
    {
      for( size_t i = 0; i < arity(); ++i )
	{
	  if( is_object( term(i) ) && (term(i) != at->term(i)) )
	    return( false );
	  else if( is_variable( term(i) ) )
	    {
	      VarSubstMap::const_iterator it = subst.find( term( i ) );
	      if( (it == subst.end()) && (term(i) != at->term(i)) )
		return( false );
	      else if( (it != subst.end()) && ((*it).second != at->term(i)) )
		return( false );
	    }
	}
      return( true );
    }
  return( false );
}

void
Atom::free_vars( VariableList &context, VariableList &vars ) const
{
  for( TermList::const_iterator ti = terms_.begin(); ti != terms_.end(); ++ti )
    if( is_variable( *ti ) )
      {
	VariableList::const_iterator vi;

	// check whether variable is in context
	for( vi = context.begin(); vi != context.end(); ++vi )
	  if( *vi == *ti ) break;
	if( vi != context.end() ) continue;

	// check whether free variable is already in list of vars
	for( vi = vars.begin(); vi != vars.end(); ++vi )
	  if( *vi == *ti ) break;
	if( vi == vars.end() ) vars.push_back( *ti );
      }
}

bool
Atom::analyze( PredicateTable &predicates, TermTable &terms,
	       std::map<const StateFormula*,const Atom*> &hash ) const
{
  return( true );
}

const StateFormula&
Atom::rewrite( std::map<const StateFormula*,const Atom*> &hash ) const
{
  Atom *atom = new Atom( predicate() );
  for( TermList::const_iterator ti = terms_.begin(); ti != terms_.end(); ++ti )
    atom->add_term( *ti );
  return( *atom );
}

void
Atom::generate_atoms( void ) const
{
  problem_t::atom_hash_get( *this );
}


/*******************************************************************************
 *
 * equality
 *
 ******************************************************************************/

Equality::Equality( Term term1, Term term2 )
  : term1_(term1), term2_(term2)
{
  StateFormula::register_use( this );
  notify( this, "Equality::Equality(Term,Term)" );
}

Equality::~Equality()
{
}

bool
Equality::holds( const AtomSet& atoms, const ValueMap& values ) const
{
  throw Exception( "Equality::holds() erroneously called" );
}

bool
Equality::holds( const state_t& state ) const
{
  throw Exception( "Equality::holds() erroneously called" );
}

const StateFormula&
Equality::flatten( bool negated ) const
{
  throw Exception( "Equality::flatten() error: unsupported formula" );
}

void
Equality::translate( atomList_t &alist ) const
{
  throw Exception( "Equality::translate() error: unsupported formula" );
}

void
Equality::translate( atomListList_t &alist ) const
{
  throw Exception( "Equality::translate() error: unsupported formula" );
}

const StateFormula&
Equality::instantiation( const SubstitutionMap& subst,
			 const problem_t& problem) const
{
  Term inst_term1 = term1_;
  if( is_variable( inst_term1 ) )
    {
      const SubstitutionMap::const_iterator si = subst.find( inst_term1 );
      if( si != subst.end() )
	inst_term1 = (*si).second;
    }

  Term inst_term2 = term2_;
  if( is_variable( inst_term2 ) )
    {
      const SubstitutionMap::const_iterator si = subst.find( inst_term2 );
      if( si != subst.end() )
	inst_term2 = (*si).second;
    }

  if( is_object( inst_term1 ) && is_object( inst_term2 ) )
    {
      if( inst_term1 == inst_term2 )
	{
	  StateFormula::register_use( &TRUE );
	  return( TRUE );
	}
      else
	{
	  StateFormula::register_use( &FALSE );
	  return( FALSE );
	}
    }
  else if( ((inst_term1 == term1_) && (inst_term2 == term2_)) ||
	   ((inst_term2 == term1_) && (inst_term1 == term2_)) )
    {
      StateFormula::register_use( this );
      return( *this );
    }
  else
    return( *new Equality( inst_term1, inst_term2 ) );
}

bool
Equality::operator==( const StateFormula &formula ) const
{
  const Equality *eq = dynamic_cast<const Equality*>( &formula );
  return( (eq != NULL) && (term1() == eq->term1()) && (term2() == eq->term2()) );
}

void
Equality::print( std::ostream& os, const PredicateTable& predicates,
		 const FunctionTable& functions, const TermTable& terms) const
{
  os << "(= ";
  terms.print_term( os, term1() );
  os << ' ';
  terms.print_term( os, term2() );
  os << ")";
}

bool
Equality::match( const StateFormula &formula, VarSubstMap &subst ) const
{
  const Equality *eq = dynamic_cast<const Equality*>( &formula );
  if( eq != NULL )
    {
      if( is_variable( term1() ) )
	{
	  VarSubstMap::const_iterator it = subst.find( term1() );
	  if( ((it == subst.end()) && (term1() != eq->term1())) ||
	      ((it != subst.end()) && ((*it).second != eq->term1())) )
	    return( false );
	}
      else if( term1() != eq->term1() )
	return( false );

      if( is_variable( term2() ) )
	{
	  VarSubstMap::const_iterator it = subst.find( term2() );
	  if( ((it == subst.end()) && (term2() != eq->term2())) ||
	      ((it != subst.end()) && ((*it).second != eq->term2())) )
	    return( false );
	}
      else if( term2() != eq->term2() )
	return( false );
    }
  return( false );
}

void
Equality::free_vars( VariableList &context, VariableList &vars ) const
{
  VariableList::const_iterator vi;
  if( is_variable( term1() ) )
    {
      // check whether term1 is in context
      for( vi = context.begin(); vi != context.end(); ++vi )
	if( *vi == term1() ) break;

      if( vi == context.end() )
	{
	  // check whether term1 is already in list of free vars
	  for( vi = vars.begin(); vi != vars.end(); ++vi )
	    if( *vi == term1() ) break;

	  if( vi == vars.end() ) vars.push_back( term1() );
	}
    }

  if( is_variable( term2() ) )
    {
      // check whether term2 is in context
      for( vi = context.begin(); vi != context.end(); ++vi )
	if( *vi == term2() ) break;

      if( vi == context.end() )
	{
	  // check whether term2 is already in list of free vars
	  for( vi = vars.begin(); vi != vars.end(); ++vi )
	    if( *vi == term2() ) break;

	  if( vi == vars.end() ) vars.push_back( term2() );
	}
    }
}

bool
Equality::analyze( PredicateTable &predicates, TermTable &terms,
		   std::map<const StateFormula*,const Atom*> &hash ) const
{
  return( true );
}

const StateFormula&
Equality::rewrite( std::map<const StateFormula*,const Atom*> &hash ) const
{
  Equality *eq = new Equality( term1(), term2() );
  return( *eq );
}

void
Equality::generate_atoms( void ) const
{
  throw Exception( "Equality::generate_atoms() erroneously called" );
}


/*******************************************************************************
 *
 * comparison
 *
 ******************************************************************************/

Comparison::Comparison( CmpPredicate predicate, const Expression& expr1,
			const Expression& expr2 )
  : predicate_(predicate), expr1_(&expr1), expr2_(&expr2)
{
  StateFormula::register_use( this );
  Expression::register_use( expr1_ );
  Expression::register_use( expr2_ );
  notify( this, "Comparison::Comparison(CmpPredicate,Expression&,Expression&)" );
}

Comparison::~Comparison()
{
  Expression::unregister_use( expr1_ );
  Expression::unregister_use( expr2_ );
}

bool
Comparison::holds( const AtomSet& atoms, const ValueMap& values ) const
{
  if( predicate() == LT_CMP )
    return( expr1().value(values) < expr2().value(values) );
  else if( predicate() == LE_CMP )
    return( expr1().value(values) <= expr2().value(values) );
  else if( predicate() == EQ_CMP )
    return( expr1().value(values) == expr2().value(values) );
  else if( predicate() == GE_CMP )
    return( expr1().value(values) >= expr2().value(values) );
  else // predicate()_ == GT_CMP
    return( expr1().value(values) > expr2().value(values) );
}

bool
Comparison::holds( const state_t& state ) const
{
  throw Exception( "Comparison::holds() erroneously called" );
}

const StateFormula&
Comparison::flatten( bool negated ) const
{
  throw Exception( "Comparison::flatten: error: unsupported formula" );
}

void
Comparison::translate( atomList_t &alist ) const
{
  throw Exception( "Comparison::translate: error: unsupported formula" );
}

void
Comparison::translate( atomListList_t &alist ) const
{
  throw Exception( "Comparison::translate: error: unsupported formula" );
}

const StateFormula&
Comparison::instantiation( const SubstitutionMap& subst, const problem_t& problem) const
{
  const Expression& inst_expr1 = expr1().instantiation( subst, problem );
  const Expression& inst_expr2 = expr2().instantiation( subst, problem );
  const Value* v1 = dynamic_cast<const Value*>( &inst_expr1 );
  if( v1 != NULL )
    {
      const Value* v2 = dynamic_cast<const Value*>( &inst_expr2 );
      if( v2 != NULL )
	{
	  bool value;
	  if( predicate() == LT_CMP )
	    value = (v1->value() < v2->value());
	  else if( predicate() == LE_CMP )
	    value = (v1->value() <= v2->value());
	  else if( predicate() == EQ_CMP )
	    value = (v1->value() == v2->value());
	  else if( predicate() == GE_CMP )
	    value = (v1->value() >= v2->value());
	  else // predicate() == GT_CMP
	    value = (v1->value() > v2->value());

	  Expression::unregister_use( v1 );
	  Expression::unregister_use( v2 );
	  if( value )
	    {
	      StateFormula::register_use( &TRUE );
	      return( TRUE );
	    }
	  else
	    {
	      StateFormula::register_use( &FALSE );
	      return( FALSE );
	    }
	}
    }
  const StateFormula *result = new Comparison( predicate(), inst_expr1, inst_expr2 );
  Expression::unregister_use( &inst_expr1 );
  Expression::unregister_use( &inst_expr2 );
  return( *result );
}

bool
Comparison::operator==( const StateFormula &formula ) const
{
  const Comparison *comp = dynamic_cast<const Comparison*>( &formula );
  return( (comp != NULL) && (predicate() == comp->predicate()) &&
	  (expr1() == comp->expr1()) && (expr2() == comp->expr2()) );
}

void
Comparison::print( std::ostream& os, const PredicateTable& predicates,
		   const FunctionTable& functions, const TermTable& terms) const
{
  os << '(';
  if( predicate() == LT_CMP )
    os << '<';
  else if( predicate() == LE_CMP )
    os << "<=";
  else if( predicate() == EQ_CMP )
    os << '=';
  else if( predicate() == GE_CMP )
    os << ">=";
  else // predicate() == GT_CMP
    os << '>';

  os << ' ';
  expr1().print( os, functions, terms );
  os << ' ';
  expr2().print( os, functions, terms );
  os << ')';
}

bool
Comparison::match( const StateFormula &formula, VarSubstMap &subst ) const
{
  throw Exception( "`Comparison::match()' not yet implemented" );
}

void
Comparison::free_vars( VariableList &context, VariableList &vars ) const
{
  throw Exception( "`Comparison::free_vars()' not yet implemented" );
}

bool
Comparison::analyze( PredicateTable &predicates, TermTable &terms,
		     std::map<const StateFormula*,const Atom*> &hash ) const
{
  return( true );
}

const StateFormula&
Comparison::rewrite( std::map<const StateFormula*,const Atom*> &hash ) const
{
  Comparison *cmp = new Comparison( predicate(), expr1(), expr2() );
  return( *cmp );
}

void
Comparison::generate_atoms( void ) const
{
  throw Exception( "Comparison::generate_atoms() erroneously called" );
}


/*******************************************************************************
 *
 * negation
 *
 ******************************************************************************/

const StateFormula&
Negation::make_negation( const StateFormula& formula )
{
  if( formula.tautology() )
    {
      StateFormula::unregister_use( &formula );
      StateFormula::register_use( &FALSE );
      return( FALSE );
    }
  else if( formula.contradiction() )
    {
      StateFormula::unregister_use( &formula );
      StateFormula::register_use( &TRUE );
      return( TRUE );
    }
  else
    {
      const StateFormula *result = new Negation( formula );
      StateFormula::unregister_use( &formula );
      return( *result );
    }
}

Negation::Negation( const StateFormula& negand )
  : negand_(&negand)
{
  StateFormula::register_use( this );
  StateFormula::register_use( negand_ );
  notify( this, "Negation::Negation(StateFormula&)" );
}

Negation::~Negation()
{
  StateFormula::unregister_use( negand_ );
}

bool
Negation::holds( const AtomSet& atoms, const ValueMap& values ) const
{
  return( !negand().holds( atoms, values ) );
}

bool
Negation::holds( const state_t& state ) const
{
  return( !negand().holds( state ) );
}

const StateFormula&
Negation::flatten( bool negated ) const
{
  return( negand().flatten( !negated ) );
}

void
Negation::translate( atomList_t &alist ) const
{
  const Atom *atom = dynamic_cast<const Atom*>( &negand() );
  if( atom != NULL )
    {
      ushort_t atm = problem_t::atom_hash_get( *atom, true );
      alist.insert( atm );
    }
  else
    throw Exception( "Negation::translate: error: unsupported formula" );
}

void
Negation::translate( atomListList_t &alist ) const
{
  atomList_t *al = new atomList_t;
  translate( *al );
  alist.insert( al );
}

const StateFormula&
Negation::instantiation( const SubstitutionMap& subst,
			 const problem_t& problem) const
{
  const StateFormula& inst_negand = negand().instantiation( subst, problem );
  if( inst_negand.contradiction() )
    {
      StateFormula::unregister_use( &inst_negand );
      StateFormula::register_use( &TRUE );
      return( TRUE );
    }
  else if( inst_negand.tautology() )
    {
      StateFormula::unregister_use( &inst_negand );
      StateFormula::register_use( &FALSE );
      return( FALSE );
    }
  else
    {
      const StateFormula *result = new Negation( inst_negand );
      StateFormula::unregister_use( &inst_negand );
      return( *result );
    }
}

bool
Negation::operator==( const StateFormula &formula ) const
{
  const Negation *neg = dynamic_cast<const Negation*>( &formula );
  return( (neg != NULL) && (negand() == neg->negand()) );
}

void
Negation::print( std::ostream& os, const PredicateTable& predicates,
		 const FunctionTable& functions, const TermTable& terms) const
{
  os << "(not ";
  negand().print( os, predicates, functions, terms );
  os << ")";
}

bool
Negation::match( const StateFormula &formula, VarSubstMap &subst ) const
{
  const Negation *neg = dynamic_cast<const Negation*>( &formula );
  return( (neg != NULL) && negand().match( neg->negand(), subst ) );
}

void
Negation::free_vars( VariableList &context, VariableList &vars ) const
{
  negand().free_vars( context, vars );
}

bool
Negation::analyze( PredicateTable &predicates, TermTable &terms,
		   std::map<const StateFormula*,const Atom*> &hash ) const
{
  return( negand().analyze( predicates, terms, hash ) );
}

const StateFormula&
Negation::rewrite( std::map<const StateFormula*,const Atom*> &hash ) const
{
  return( Negation::make_negation( negand().rewrite( hash ) ) );
}

void
Negation::generate_atoms( void ) const
{
  negand().generate_atoms();
}


/*******************************************************************************
 *
 * conjunction
 *
 ******************************************************************************/

Conjunction::Conjunction()
{
  StateFormula::register_use( this );
  notify( this, "Conjunction::Conjunction()" );
}

Conjunction::~Conjunction()
{
  for( FormulaList::const_iterator fi = conjuncts_.begin(); fi != conjuncts_.end(); ++fi )
    StateFormula::unregister_use( *fi );
}

void
Conjunction::cross_products( std::list<const Disjunction*>::const_iterator it,
			     std::list<const Disjunction*>::const_iterator end,
			     std::list<const StateFormula*> &flist,
			     std::list<const Conjunction*> &xproduct )
{
  if( it == end )
    {
      Conjunction *conj = new Conjunction;
      for( std::list<const StateFormula*>::const_iterator i = flist.begin(); i != flist.end(); ++i )
	{
	  if( dynamic_cast<const Conjunction*>( (*i) ) )
	    {
	      const Conjunction *f = (const Conjunction*)(*i);
	      for( size_t j = 0; j < f->size(); ++j )
		{
		  StateFormula::register_use( &f->conjunct( j ) );
		  conj->add_conjunct( f->conjunct( j ) );
		}
	    }
	  else
	    {
	      StateFormula::register_use( (*i) );
	      conj->add_conjunct( *(*i) );
	    }
	}
      xproduct.push_back( conj );
    }
  else
    {
      for( size_t i = 0; i < (*it)->size(); ++i )
	{
	  flist.push_back( &(*it)->disjunct( i ) );
	  ++it;
	  cross_products( it, end, flist, xproduct );
	  --it;
	  flist.pop_back();
	}
    }
}

const Conjunction&
Conjunction::merge( const Conjunction &conjunction ) const
{
  Conjunction *result = new Conjunction;
  for( size_t i = 0; i < size(); ++i )
    {
      StateFormula::register_use( &conjunct( i ) );
      result->add_conjunct( conjunct( i ) );
    }

  for( size_t i = 0; i < conjunction.size(); ++i )
    {
      StateFormula::register_use( &conjunction.conjunct( i ) );
      result->add_conjunct( conjunction.conjunct( i ) );
    }
  return( *result );
}

void
Conjunction::add_conjunct( const StateFormula& conjunct )
{
  if( !conjunct.tautology() )
    conjuncts_.push_back( &conjunct );
  else
    StateFormula::unregister_use( &conjunct );
}

bool
Conjunction::holds( const AtomSet& atoms, const ValueMap& values ) const
{
  for( FormulaList::const_iterator fi = conjuncts_.begin(); fi != conjuncts_.end(); ++fi )
    if( !(*fi)->holds( atoms, values ) )
      return( false );
  return( true );
}

bool
Conjunction::holds( const state_t& state ) const
{
  for( FormulaList::const_iterator fi = conjuncts_.begin(); fi != conjuncts_.end(); ++fi )
    if( !(*fi)->holds( state ) ) return( false );
  return( true );
}

const StateFormula&
Conjunction::flatten( bool negated ) const
{
  std::list<const Disjunction*> disj_list;
  std::list<const Conjunction*> conj_list;
  std::list<const StateFormula*> other_list;

  // base cases
  if( size() == 0 )
    {
      StateFormula::register_use( &FALSE );
      return( FALSE );
    }
  else if( size() == 1 )
    {
      return( conjunct( 0 ).flatten( negated ) );
    }

  // flat conjuncts and classify into disjunctions, conjunctions, and others
  for( size_t i = 0; i < size(); ++i )
    {
      const StateFormula *flat = &conjunct( i ).flatten( negated );
      if( dynamic_cast<const Disjunction*>( flat ) )
	disj_list.push_back( (const Disjunction*)flat );
      else if( dynamic_cast<const Conjunction*>( flat ) )
	conj_list.push_back( (const Conjunction*)flat );
      else
	other_list.push_back( flat );
    }

  if( !negated )
    {
      // collect conjunctions and others into a conjunction
      Conjunction *conj = new Conjunction;

      for( std::list<const Conjunction*>::const_iterator it = conj_list.begin(); it != conj_list.end(); ++it )
	{
	  for( size_t i = 0; i < (*it)->size(); ++i )
	    conj->add_conjunct( (*it)->conjunct( i ) );
	}

      for( std::list<const StateFormula*>::const_iterator it = other_list.begin(); it != other_list.end(); ++it )
	conj->add_conjunct( *(*it) );

      if( disj_list.size() == 0 )
	{
	  // result is a conjunction
	  return( *conj );
	}
      else
	{
	  // result is a disjunction
	  Disjunction *result = new Disjunction;

	  // apply distribution laws to form cross products
	  std::list<const Conjunction*> xproduct;
	  std::list<const StateFormula*> flist;
	  Conjunction::cross_products( disj_list.begin(), disj_list.end(), flist, xproduct );

	  // insert cross products into result
	  for( std::list<const Conjunction*>::const_iterator it = xproduct.begin(); it != xproduct.end(); ++it )
	    {
	      result->add_disjunct( conj->merge( *(*it) ) );
	      StateFormula::unregister_use( (*it) );
	    }
	  StateFormula::unregister_use( conj );

	  // return
	  return( *result );
	}
    }
  else
    {
      // result is a big disjunction
      Disjunction *result = new Disjunction;

      for( std::list<const Disjunction*>::const_iterator it = disj_list.begin(); it != disj_list.end(); ++it )
	{
	  for( size_t i = 0; i < (*it)->size(); ++i )
	    result->add_disjunct( (*it)->disjunct( i ) );
	}

      for( std::list<const Conjunction*>::const_iterator it = conj_list.begin(); it != conj_list.end(); ++it )
	result->add_disjunct( *(*it) );

      for( std::list<const StateFormula*>::const_iterator it = other_list.begin(); it != other_list.end(); ++it )
	result->add_disjunct( *(*it) );

      // return
      return( *result );
    }
}

void
Conjunction::translate( atomList_t &alist ) const
{
  for( size_t i = 0; i < size(); ++i )
    conjunct( i ).translate( alist );
}

void
Conjunction::translate( atomListList_t &alist ) const
{
  atomList_t *al = new atomList_t;
  translate( *al );
  alist.insert( al );
}

const StateFormula&
Conjunction::instantiation( const SubstitutionMap& subst,
			    const problem_t& problem) const
{
  Conjunction* conj = new Conjunction;
  for( FormulaList::const_iterator fi = conjuncts_.begin(); fi != conjuncts_.end(); ++fi )
    {
      const StateFormula& c = (*fi)->instantiation( subst, problem );
      if( c.contradiction() )
	{
	  StateFormula::unregister_use( &c );
	  StateFormula::unregister_use( conj );
	  StateFormula::register_use( &FALSE );
	  return( FALSE );
	}
      else if( !c.tautology() )
	{
	  conj->add_conjunct( c );
	}
      else
	StateFormula::unregister_use( &c );
    }

  if( conj->size() == 0 )
    {
      StateFormula::unregister_use( conj );
      StateFormula::register_use( &TRUE );
      return( TRUE );
    }
  if( conj->size() == 1 )
    {
      const StateFormula *result = &conj->conjunct( 0 );
      StateFormula::register_use( result );
      StateFormula::unregister_use( conj );
      return( *result );
    }
  else
    {
      return( *conj );
    }
}

bool
Conjunction::operator==( const StateFormula &formula ) const
{
  const Conjunction *conj = dynamic_cast<const Conjunction*>( &formula );
  if( (conj != NULL) && (size() == conj->size()) )
    {
      for( size_t i = 0; i < size(); ++i )
	if( !(conjunct( i ) == conj->conjunct( i )) )
	  return( false );
      return( true );
    }
  return( false );
}

void
Conjunction::print( std::ostream& os, const PredicateTable& predicates,
		    const FunctionTable& functions, const TermTable& terms ) const
{
  if( size() == 1 )
    {
      conjunct( 0 ).print( os, predicates, functions, terms );
    }
  else
    {
      os << "(and";
      for( FormulaList::const_iterator fi = conjuncts_.begin(); fi != conjuncts_.end(); ++fi )
	{
	  os << ' ';
	  (*fi)->print( os, predicates, functions, terms );
	}
      os << ")";
    }
}

bool
Conjunction::match( const StateFormula &formula, VarSubstMap &subst ) const
{
  const Conjunction *conj = dynamic_cast<const Conjunction*>( &formula );
  if( (conj != NULL) && (size() == conj->size()) )
    {
      for( size_t i = 0; i < size(); ++i )
	if( !conjunct( i ).match( conj->conjunct( i ), subst ) )
	  return( false );
      return( true );
    }
  return( false );
}

void
Conjunction::free_vars( VariableList &context, VariableList &vars ) const
{
  for( FormulaList::const_iterator fi = conjuncts_.begin(); fi != conjuncts_.end(); ++fi )
    (*fi)->free_vars( context, vars );
}

bool
Conjunction::analyze( PredicateTable &predicates, TermTable &terms,
		      std::map<const StateFormula*,const Atom*> &hash ) const
{
  bool rv = true;
  for( FormulaList::const_iterator fi = conjuncts_.begin(); fi != conjuncts_.end(); ++fi )
    rv = (*fi)->analyze( predicates, terms, hash ) && rv;
  return( rv );
}

const StateFormula&
Conjunction::rewrite( std::map<const StateFormula*,const Atom*> &hash ) const
{
  Conjunction *conj = new Conjunction;
  for( FormulaList::const_iterator fi = conjuncts_.begin(); fi != conjuncts_.end(); ++fi )
    conj->add_conjunct( (*fi)->rewrite( hash ) );
  return( *conj );
}

void
Conjunction::generate_atoms( void ) const
{
  for( FormulaList::const_iterator fi = conjuncts_.begin(); fi != conjuncts_.end(); ++fi )
    (*fi)->generate_atoms();
}


/*******************************************************************************
 *
 * disjunction
 *
 ******************************************************************************/

Disjunction::Disjunction()
{
  StateFormula::register_use( this );
  notify( this, "Disjunction::Disjunction()" );
}

Disjunction::~Disjunction()
{
  for( FormulaList::const_iterator fi = disjuncts_.begin(); fi != disjuncts_.end(); ++fi )
    StateFormula::unregister_use( *fi );
}

void
Disjunction::cross_products( std::list<const Conjunction*>::const_iterator it,
			     std::list<const Conjunction*>::const_iterator end,
			     std::list<const StateFormula*> &flist,
			     std::list<const Disjunction*> &xproduct )
{
  if( it == end )
    {
      Disjunction *disj = new Disjunction;
      for( std::list<const StateFormula*>::const_iterator i = flist.begin(); i != flist.end(); ++i )
	{
	  if( dynamic_cast<const Disjunction*>( (*i) ) )
	    {
	      const Disjunction *f = (const Disjunction*)(*i);
	      for( size_t j = 0; j < f->size(); ++j )
		{
		  StateFormula::register_use( &f->disjunct( j ) );
		  disj->add_disjunct( f->disjunct( j ) );
		}
	    }
	  else
	    {
	      StateFormula::register_use( (*i) );
	      disj->add_disjunct( *(*i) );
	    }
	}
      xproduct.push_back( disj );
    }
  else
    {
      for( size_t i = 0; i < (*it)->size(); ++i )
	{
	  flist.push_back( &(*it)->conjunct( i ) );
	  ++it;
	  cross_products( it, end, flist, xproduct );
	  --it;
	  flist.pop_back();
	}
    }
}

const Disjunction&
Disjunction::merge( const Disjunction &disjunction ) const
{
  Disjunction *result = new Disjunction;
  for( size_t i = 0; i < size(); ++i )
    {
      StateFormula::register_use( &disjunct( i ) );
      result->add_disjunct( disjunct( i ) );
    }

  for( size_t i = 0; i < disjunction.size(); ++i )
    {
      StateFormula::register_use( &disjunction.disjunct( i ) );
      result->add_disjunct( disjunction.disjunct( i ) );
    }
  return( *result );
}

void
Disjunction::add_disjunct( const StateFormula& disjunct )
{
  if( !disjunct.contradiction() )
    disjuncts_.push_back( &disjunct );
  else
    StateFormula::unregister_use( &disjunct );
}

bool
Disjunction::holds( const AtomSet& atoms, const ValueMap& values ) const
{
  for( FormulaList::const_iterator fi = disjuncts_.begin(); fi != disjuncts_.end(); ++fi )
    if( (*fi)->holds( atoms, values ) )
      return( true );
  return( false );
}

bool
Disjunction::holds( const state_t& state ) const
{
  for( FormulaList::const_iterator fi = disjuncts_.begin(); fi != disjuncts_.end(); ++fi )
    if( (*fi)->holds( state ) ) return( true );
  return( false );
}

const StateFormula&
Disjunction::flatten( bool negated ) const
{
  std::list<const Disjunction*> disj_list;
  std::list<const Conjunction*> conj_list;
  std::list<const StateFormula*> other_list;

  // base cases
  if( size() == 0 )
    {
      StateFormula::register_use( &TRUE );
      return( TRUE );
    }
  else if( size() == 1 )
    {
      return( disjunct( 0 ).flatten( negated ) );
    }

  // flat disjuncts and classify into disjunctions, conjunctions, and others
  for( size_t i = 0; i < size(); ++i )
    {
      const StateFormula *flat = &disjunct( i ).flatten( negated );
      if( dynamic_cast<const Disjunction*>( flat ) )
	disj_list.push_back( (const Disjunction*)flat );
      else if( dynamic_cast<const Conjunction*>( flat ) )
	conj_list.push_back( (const Conjunction*)flat );
      else
	other_list.push_back( flat );
    }

  if( !negated )
    {
      // result is a big disjunction
      Disjunction *result = new Disjunction;

      for( std::list<const Disjunction*>::const_iterator it = disj_list.begin(); it != disj_list.end(); ++it )
	{
	  for( size_t i = 0; i < (*it)->size(); ++i )
	    result->add_disjunct( (*it)->disjunct( i ) );
	}

      for( std::list<const Conjunction*>::const_iterator it = conj_list.begin(); it != conj_list.end(); ++it )
	result->add_disjunct( *(*it) );

      for( std::list<const StateFormula*>::const_iterator it = other_list.begin(); it != other_list.end(); ++it )
	result->add_disjunct( *(*it) );

      // return
      return( *result );
    }
  else
    {
      // collect conjunctions and others into a conjunction
      Conjunction *conj = new Conjunction;

      for( std::list<const Conjunction*>::const_iterator it = conj_list.begin(); it != conj_list.end(); ++it )
	{
	  for( size_t i = 0; i < (*it)->size(); ++i )
	    conj->add_conjunct( (*it)->conjunct( i ) );
	}

      for( std::list<const StateFormula*>::const_iterator it = other_list.begin(); it != other_list.end(); ++it )
	conj->add_conjunct( *(*it) );

      if( disj_list.size() == 0 )
	{
	  // result is a conjunction
	  return( *conj );
	}
      else
	{
	  // result is a disjunction
	  Disjunction *result = new Disjunction;

	  // apply distribution laws to form cross products
	  std::list<const Conjunction*> xproduct;
	  std::list<const StateFormula*> flist;
	  Conjunction::cross_products( disj_list.begin(), disj_list.end(), flist, xproduct );

	  // insert cross products into result
	  for( std::list<const Conjunction*>::const_iterator it = xproduct.begin(); it != xproduct.end(); ++it )
	    {
	      result->add_disjunct( conj->merge( *(*it) ) );
	      StateFormula::unregister_use( (*it) );
	    }
	  StateFormula::unregister_use( conj );

	  // return
	  return( *result );
	}
    }
}

void
Disjunction::translate( atomList_t &alist ) const
{
  throw Exception( "Disjunction::translate: error: unsupported formula" );
}

void
Disjunction::translate( atomListList_t &alist ) const
{
  for( size_t i = 0; i < size(); ++i )
    {
      atomList_t *al = new atomList_t;
      disjunct( i ).translate( *al );
      alist.insert( al );
    }
}

const StateFormula&
Disjunction::instantiation( const SubstitutionMap& subst,
			    const problem_t& problem) const
{
  Disjunction* disj = new Disjunction;
  for( FormulaList::const_iterator fi = disjuncts_.begin(); fi != disjuncts_.end(); ++fi )
    {
      const StateFormula& d = (*fi)->instantiation( subst, problem );
      if( d.tautology() )
	{
	  StateFormula::unregister_use( &d );
	  StateFormula::unregister_use( disj );
	  StateFormula::register_use( &TRUE );
	  return( TRUE );
	}
      else if( !d.contradiction() )
	{
	  disj->add_disjunct( d );
	}
      else
	StateFormula::unregister_use( &d );
    }

  if( disj->size() == 0 )
    {
      StateFormula::unregister_use( disj );
      StateFormula::register_use( &FALSE );
      return( FALSE );
    }
  if( disj->size() == 1 )
    {
      const StateFormula *result = &disj->disjunct( 0 );
      StateFormula::register_use( result );
      StateFormula::unregister_use( disj );
      return( *result );
    }
  else
    {
      return( *disj );
    }
}

bool
Disjunction::operator==( const StateFormula &formula ) const
{
  const Disjunction *disj = dynamic_cast<const Disjunction*>( &formula );
  if( (disj != NULL) && (size() == disj->size()) )
    {
      for( size_t i = 0; i < size(); ++i )
	if( !(disjunct( i ) == disj->disjunct( i )) )
	  return( false );
      return( true );
    }
  return( false );
}

void
Disjunction::print( std::ostream& os, const PredicateTable& predicates,
		    const FunctionTable& functions, const TermTable& terms ) const
{
  if( size() == 1 )
    disjunct( 0 ).print( os, predicates, functions, terms );
  else
    {
      os << "(or";
      for( FormulaList::const_iterator fi = disjuncts_.begin(); fi != disjuncts_.end(); ++fi )
	{
	  os << ' ';
	  (*fi)->print( os, predicates, functions, terms );
	}
      os << ")";
    }
}

bool
Disjunction::match( const StateFormula &formula, VarSubstMap &subst ) const
{
  const Disjunction *disj = dynamic_cast<const Disjunction*>( &formula );
  if( (disj != NULL) && (size() == disj->size()) )
    {
      for( size_t i = 0; i < size(); ++i )
	if( !disjunct( i ).match( disj->disjunct( i ), subst ) )
	  return( false );
      return( true );
    }
  return( false );
}

void
Disjunction::free_vars( VariableList &context, VariableList &vars ) const
{
  for( FormulaList::const_iterator fi = disjuncts_.begin(); fi != disjuncts_.end(); ++fi )
    (*fi)->free_vars( context, vars );
}

bool
Disjunction::analyze( PredicateTable &predicates, TermTable &terms,
		      std::map<const StateFormula*,const Atom*> &hash ) const
{
  bool rv = true;
  for( FormulaList::const_iterator fi = disjuncts_.begin(); fi != disjuncts_.end(); ++fi )
    rv = (*fi)->analyze( predicates, terms, hash ) && rv;
  return( rv );
}

const StateFormula&
Disjunction::rewrite( std::map<const StateFormula*,const Atom*> &hash ) const
{
  Disjunction *disj = new Disjunction;
  for( FormulaList::const_iterator fi = disjuncts_.begin(); fi != disjuncts_.end(); ++fi )
    disj->add_disjunct( (*fi)->rewrite( hash ) );
  return( *disj );
}

void
Disjunction::generate_atoms( void ) const
{
  for( FormulaList::const_iterator fi = disjuncts_.begin(); fi != disjuncts_.end(); ++fi )
    (*fi)->generate_atoms();
}


/*******************************************************************************
 *
 * exists
 *
 ******************************************************************************/

Exists::Exists()
  : body_(&StateFormula::FALSE)
{
  StateFormula::register_use( this );
  StateFormula::register_use( body_ );
  notify( this, "Exists::Exists()" );
}

Exists::~Exists()
{
  StateFormula::unregister_use( body_ );
}

void
Exists::set_body( const StateFormula& body )
{
  if( &body != body_ )
    {
      StateFormula::unregister_use( body_ );
      body_ = &body;
      StateFormula::register_use( body_ );
    }
}

bool
Exists::holds( const AtomSet& atoms, const ValueMap& values ) const
{
  throw Exception( "Exists::holds() erroneously called" );
}

bool
Exists::holds( const state_t& state ) const
{
  throw Exception( "Exists::holds() erroneously called" );
}

const StateFormula&
Exists::flatten( bool negated ) const
{
  throw Exception( "Exists::flatten: error: unsupported formula" );
}

void
Exists::translate( atomList_t &alist ) const
{
  throw Exception( "Exists::translate: error: unsupported formula" );
}

void
Exists::translate( atomListList_t &alist ) const
{
  throw Exception( "Exists::translate: error: unsupported formula" );
}

const StateFormula&
Exists::instantiation( const SubstitutionMap& subst,
		       const problem_t& problem ) const
{
  int n = arity();
  if( n == 0 )
    {
      return( body().instantiation( subst, problem ) );
    }
  else
    {
      SubstitutionMap args( subst );
      std::vector<ObjectList> arguments( n, ObjectList() );
      std::vector<ObjectList::const_iterator> next_arg;
      for( int i = 0; i < n; ++i )
	{
	  problem.compatible_objects( arguments[i], problem.terms().type(parameter(i)) );
	  if( arguments[i].empty() )
	    return( FALSE );
	  next_arg.push_back( arguments[i].begin() );
	}

      Disjunction* disj = new Disjunction;
      std::stack<const StateFormula*> disjuncts;
      disjuncts.push( &body().instantiation( args, problem ) );

      for( int i = 0; i < n; )
	{
	  SubstitutionMap pargs;
	  pargs.insert( std::make_pair( parameter(i), *next_arg[i] ) );
	  const StateFormula& disjunct = disjuncts.top()->instantiation( pargs, problem );
	  disjuncts.push( &disjunct );

	  if( i + 1 == n )
	    {
	      if( disjunct.tautology() )
		{
		  StateFormula::unregister_use( disj );
		  disj = NULL;
		  break;
		}
	      else if( !disjunct.contradiction() )
		{
		  disj->add_disjunct( disjunct );
		  StateFormula::register_use( &disjunct );
		}

	      for( int j = n - 1; j >= 0; --j )
		{
		  StateFormula::unregister_use( disjuncts.top() );
		  disjuncts.pop();
		  next_arg[j]++;
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
	      ++i;
	    }
	}

      while( !disjuncts.empty() )
	{
	  StateFormula::unregister_use( disjuncts.top() );
	  disjuncts.pop();
	}

      if( disj != NULL )
	{
	  if( disj->size() == 1 )
	    {
	      const StateFormula *result = &disj->disjunct( 0 );
	      StateFormula::register_use( result );
	      StateFormula::unregister_use( disj );
	      return( *result );
	    }
	  else
	    return( *disj );
	}
      else
	{
	  StateFormula::register_use( &TRUE );
	  return( TRUE );
	}
    }
  return( *this ); // to remove a g++ warning
}

bool
Exists::operator==( const StateFormula &formula ) const
{
  const Exists *ex = dynamic_cast<const Exists*>( &formula );
  if( ex != NULL )
    {
      return( (parameters_ == ex->parameters_) && (body() == ex->body()) );
    }
  return( false );
}

void
Exists::print( std::ostream& os, const PredicateTable& predicates,
	       const FunctionTable& functions, const TermTable& terms) const
{
  if( parameters_.empty() )
    body().print( os, predicates, functions, terms );
  else
    {
      os << "(exists (";
      VariableList::const_iterator vi = parameters_.begin();
      terms.print_term( os, *vi );
      for( ++vi; vi != parameters_.end(); ++vi )
	{
	  os << ' ';
	  terms.print_term( os, *vi );
	}
      os << ") ";
      body().print( os, predicates, functions, terms );
      os << ")";
    }
}

bool
Exists::match( const StateFormula &formula, VarSubstMap &subst ) const
{
  const Exists *ex = dynamic_cast<const Exists*>( &formula );
  if( (ex != NULL) && (parameters_.size() == ex->parameters_.size()) )
    {
      // copy subs map and extend
      VarSubstMap nsubst = subst;
      for( size_t i = 0; i < arity(); ++i )
	nsubst[parameter(i)] = ex->parameter(i);

      // match bodies
      return( body().match( ex->body(), nsubst ) );
    }
  return( false );
}

void
Exists::free_vars( VariableList &context, VariableList &vars ) const
{
  // extend context
  for( VariableList::const_iterator vi = parameters_.begin(); vi != parameters_.end(); ++vi )
    context.push_back( *vi );

  body().free_vars( context, vars );

  // shrink context
  for( size_t i = 0; i < parameters_.size(); ++i )
    context.pop_back();
}

bool
Exists::analyze( PredicateTable &predicates, TermTable &terms,
		 std::map<const StateFormula*,const Atom*> &hash ) const
{
  if( body().analyze( predicates, terms, hash ) )
    {
      VarSubstMap subst;
      VariableList context, fvars1, fvars2;
      free_vars( context, fvars1 );

      std::map<const StateFormula*,const Atom*>::iterator hi;
      for( hi = hash.begin(); hi != hash.end(); ++hi )
	{
	  context.clear();
	  (*hi).first->free_vars( context, fvars2 );

	  if( fvars1.size() == fvars2.size() )
	    {
	      subst.clear();
	      for( size_t i = 0; i < fvars1.size(); ++i )
		subst[fvars1[i]] = fvars2[i];

	      if( match( *(*hi).first, subst ) )
		{
		  // create new atom
		  Predicate p = (*hi).second->predicate();
		  const Atom *atom = &Atom::make_atom( p, *(TermList*)&fvars1 );

		  // insert formula and atom into hash
		  StateFormula::register_use( this );
		  hash.insert( std::make_pair( this, atom ) );
		  break;
		}
	    }
	}

      if( hi == hash.end() )
	{
	  // create new predicate
	  std::ostringstream ost;
	  ost << "*internal-" << ++internal_predicate_index << "*";
	  Predicate p = predicates.add_predicate( ost.str() );

	  for( VariableList::const_iterator vi = fvars1.begin(); vi != fvars1.end(); ++vi )
	    predicates.add_parameter( p, terms.type( *vi ) );

	  // create new atom
	  const Atom *atom = &Atom::make_atom( p, *(TermList*)&fvars1 );

	  // insert formula and atom into hash
	  StateFormula::register_use( this );
	  hash.insert( std::make_pair( this, atom ) );
	}
    }
  return( false );
}

const StateFormula&
Exists::rewrite( std::map<const StateFormula*,const Atom*> &hash ) const
{
  std::map<const StateFormula*,const Atom*>::const_iterator hi = hash.find( this );
  if( hi != hash.end() )
    {
      StateFormula::register_use( (*hi).second );
      return( *(*hi).second );
    }
  else
    {
      Exists *ex = new Exists;
      ex->parameters_ = parameters_;
      ex->body_ = &body().rewrite( hash );
      return( *ex );
    }
}

void
Exists::generate_atoms( void ) const
{
  throw Exception( "Exists::generate_atoms() erroneously called" );
}


/*******************************************************************************
 *
 * forall
 *
 ******************************************************************************/

Forall::Forall()
  : body_(&StateFormula::TRUE)
{
  StateFormula::register_use( this );
  StateFormula::register_use( body_ );
  notify( this, "Forall::Forall()" );
}

Forall::~Forall()
{
  StateFormula::unregister_use( body_ );
}

void
Forall::set_body( const StateFormula& body )
{
  if( &body != body_ )
    {
      StateFormula::unregister_use( body_ );
      body_ = &body;
      StateFormula::register_use( body_ );
    }
}

bool
Forall::holds( const AtomSet& atoms, const ValueMap& values ) const
{
  throw Exception( "Forall::holds() erroneously called" );
}

bool
Forall::holds( const state_t& state ) const
{
  throw Exception( "Forall::holds() erroneously called" );
}

const StateFormula&
Forall::flatten( bool negated ) const
{
  throw Exception( "Forall::flatten: error: unsupported formula" );
}

void
Forall::translate( atomList_t &alist ) const
{
  throw Exception( "Forall::translate: error: unsupported formula" );
}

void
Forall::translate( atomListList_t &alist ) const
{
  throw Exception( "Forall::translate: error: unsupported formula" );
}

const StateFormula&
Forall::instantiation(const SubstitutionMap& subst,
		      const problem_t& problem) const
{
  int n = arity();
  if( n == 0 )
    {
      return( body().instantiation( subst, problem ) );
    }
  else
    {
      SubstitutionMap args( subst );
      std::vector<ObjectList> arguments( n, ObjectList() );
      std::vector<ObjectList::const_iterator> next_arg;

      for( int i = 0; i < n; ++i )
	{
	  problem.compatible_objects( arguments[i], problem.terms().type(parameter(i)) );
	  if( arguments[i].empty() )
	    return( TRUE );
	  next_arg.push_back( arguments[i].begin() );
	}

      Conjunction* conj = new Conjunction;
      std::stack<const StateFormula*> conjuncts;
      conjuncts.push( &body().instantiation( args, problem ) );

      for( int i = 0; i < n; )
	{
	  SubstitutionMap pargs;
	  pargs.insert( std::make_pair( parameter(i), *next_arg[i] ) );
	  const StateFormula& conjunct = conjuncts.top()->instantiation( pargs, problem );
	  conjuncts.push( &conjunct );

	  if( i + 1 == n )
	    {
	      if( conjunct.contradiction() )
		{
		  StateFormula::unregister_use( conj );
		  conj = NULL;
		  break;
		}
	      else if( !conjunct.tautology() )
		{
		  conj->add_conjunct( conjunct );
		  StateFormula::register_use( &conjunct );
		}

	      for( int j = n - 1; j >= 0; --j )
		{
		  StateFormula::unregister_use( conjuncts.top() );
		  conjuncts.pop();
		  next_arg[j]++;
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
	      ++i;
	    }
	}

      while( !conjuncts.empty() )
	{
	  StateFormula::unregister_use( conjuncts.top() );
	  conjuncts.pop();
	}

      if( conj != NULL )
	{
	  if( conj->size() == 1 )
	    {
	      const StateFormula *result = &conj->conjunct( 0 );
	      StateFormula::register_use( result );
	      StateFormula::unregister_use( conj );
	      return( *result );
	    }
	  else
	    return( *conj );
	}
      else
	{
	  StateFormula::register_use( &FALSE );
	  return( FALSE );
	}
    }
  return( *this ); // to remove a g++ warning
}

bool
Forall::operator==( const StateFormula &formula ) const
{
  const Forall *fo = dynamic_cast<const Forall*>( &formula );
  if( fo != NULL )
    {
      return( (parameters_ == fo->parameters_) && (body() == fo->body()) );
    }
  return( false );
}

void
Forall::print( std::ostream& os, const PredicateTable& predicates,
	       const FunctionTable& functions, const TermTable& terms ) const
{
  if( parameters_.empty() )
    body().print( os, predicates, functions, terms );
  else
    {
      os << "(forall (";
      VariableList::const_iterator vi = parameters_.begin();
      terms.print_term( os, *vi );
      for ( ++vi; vi != parameters_.end(); ++vi )
	{
	  os << ' ';
	  terms.print_term( os, *vi );
	}
      os << ") ";
      body().print( os, predicates, functions, terms );
      os << ")";
    }
}

bool
Forall::match( const StateFormula &formula, VarSubstMap &subst ) const
{
  const Forall *fo = dynamic_cast<const Forall*>( &formula );
  if( (fo != NULL) && (parameters_.size() == fo->parameters_.size()) )
    {
      // copy subs map and extend
      VarSubstMap nsubst = subst;
      for( size_t i = 0; i < arity(); ++i )
	nsubst[parameter(i)] = fo->parameter(i);

      // match bodies
      return( body().match( fo->body(), nsubst ) );
    }
  return( false );
}

void
Forall::free_vars( VariableList &context, VariableList &vars ) const
{
  // extend context
  for( VariableList::const_iterator vi = parameters_.begin(); vi != parameters_.end(); ++vi )
    context.push_back( *vi );

  body().free_vars( context, vars );

  // shrink context
  for( size_t i = 0; i < parameters_.size(); ++i )
    context.pop_back();
}

bool
Forall::analyze( PredicateTable &predicates, TermTable &terms,
		 std::map<const StateFormula*,const Atom*> &hash ) const
{
  if( body().analyze( predicates, terms, hash ) )
    {
      VarSubstMap subst;
      VariableList context, fvars1, fvars2;
      free_vars( context, fvars1 );

      std::map<const StateFormula*,const Atom*>::iterator hi;
      for( hi = hash.begin(); hi != hash.end(); ++hi )
	{
	  context.clear();
	  (*hi).first->free_vars( context, fvars2 );

	  if( fvars1.size() == fvars2.size() )
	    {
	      subst.clear();
	      for( size_t i = 0; i < fvars1.size(); ++i )
		subst[fvars1[i]] = fvars2[i];

	      if( match( *(*hi).first, subst ) )
		{
		  // create new atom
		  Predicate p = (*hi).second->predicate();
		  const Atom *atom = &Atom::make_atom( p, *(TermList*)&fvars1 );

		  // insert formula and atom into hash
		  StateFormula::register_use( this );
		  hash.insert( std::make_pair( this, atom ) );
		  break;
		}
	    }
	}

      if( hi == hash.end() )
	{
	  // create new predicate
	  std::ostringstream ost;
	  ost << "*internal-" << ++internal_predicate_index << "*";
	  Predicate p = predicates.add_predicate( ost.str() );
	  predicates.make_dynamic( p );

	  for( VariableList::const_iterator vi = fvars1.begin(); vi != fvars1.end(); ++vi )
	    predicates.add_parameter( p, terms.type( *vi ) );

	  // create new atom
	  const Atom *atom = &Atom::make_atom( p, *(TermList*)&fvars1 );

	  // insert formula and atom into hash
	  StateFormula::register_use( this );
	  hash.insert( std::make_pair( this, atom ) );
	}
    }
  return( false );
}

const StateFormula&
Forall::rewrite( std::map<const StateFormula*,const Atom*> &hash ) const
{
  std::map<const StateFormula*,const Atom*>::const_iterator hi = hash.find( this );
  if( hi != hash.end() )
    {
      StateFormula::register_use( (*hi).second );
      return( *(*hi).second );
    }
  else
    {
      Forall *fo = new Forall;
      fo->parameters_ = parameters_;
      fo->body_ = &body().rewrite( hash );
      return( *fo );
    }
}

void
Forall::generate_atoms( void ) const
{
  throw Exception( "Forall::generate_atoms() erroneously called" );
}
