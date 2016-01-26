#include "expressions.h"
#include "problems.h"
#include "domains.h"
#include "exceptions.h"


/*******************************************************************************
 *
 * value
 *
 ******************************************************************************/

Rational
Value::value( const ValueMap& values ) const
{
  return( value_ );
}

const Value& 
Value::instantiation( const SubstitutionMap& subst, const problem_t& problem ) const
{
  return( *this );
}

bool
Value::operator==( const Expression &expr ) const
{
  const Value *vexpr = dynamic_cast<const Value*>( &expr );
  return( (vexpr != NULL) && (value() == vexpr->value()) );

}

void
Value::print( std::ostream& os, const FunctionTable& functions, const TermTable& terms ) const
{
  os << value_;
}


/*******************************************************************************
 *
 * application
 *
 ******************************************************************************/

Application::ApplicationTable Application::applications;

bool
Application::ApplicationLess::operator()( const Application* a1,
					  const Application* a2 ) const
{
  if( a1->function() < a2->function() )
    return( true );
  else if( a1->function() > a2->function() )
    return( false );
  else
    {
      for( size_t i = 0; i < a1->arity(); ++i )
	{
	  if( a1->term(i) < a2->term( i ) )
	    return( true );
	  else if( a1->term(i) > a2->term( i ) )
	    return( false );
	}
      return( false );
    }
}

const Application&
Application::make_application( Function function, const TermList& terms )
{
  Application* application = new Application( function );
  bool ground = true;
  for( TermList::const_iterator ti = terms.begin(); ti != terms.end(); ++ti )
    {
      application->add_term( *ti );
      if( ground && is_variable( *ti ) )
	ground = false;
    }
  if( !ground )
    return( *application );
  else
    {
      std::pair<ApplicationTable::const_iterator,bool> result = applications.insert( application );
      if( !result.second )
	{
	  delete application;
	  return( **result.first );
	}
      else
      return( *application );
    }
}

Application::~Application()
{
  ApplicationTable::const_iterator ai = applications.find( this );
  if( *ai == this )
    applications.erase( ai );
}

Rational
Application::value( const ValueMap& values ) const
{
  ValueMap::const_iterator vi = values.find( this );
  if( vi != values.end() )
    return( (*vi).second );
  else
    throw Exception( "value of function application undefined" );
}

const Application&
Application::substitution( const SubstitutionMap& subst ) const
{
  TermList inst_terms;
  bool substituted = false;
  for( TermList::const_iterator ti = terms_.begin(); ti != terms_.end(); ++ti )
    {
      SubstitutionMap::const_iterator si = is_variable(*ti) ? subst.find(*ti) : subst.end();
      if( si != subst.end() )
	{
	  inst_terms.push_back( (*si).second );
	  substituted = true;
	}
      else
	inst_terms.push_back( *ti );
    }
  if( substituted )
    return( make_application( function(), inst_terms ) );
  else 
    return( *this );
}

const Expression& 
Application::instantiation( const SubstitutionMap& subst, const problem_t& problem ) const
{
  TermList inst_terms;
  bool substituted = false;
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
	  if( is_object( *ti ) ) ++objects;
	}
    }

  if( substituted )
    {
      const Application& inst_appl = make_application( function(), inst_terms );
      if( problem.domain().functions().static_function( function() ) &&
	  (objects == inst_terms.size()) )
	{
	  ValueMap::const_iterator vi = problem.init_fluents().find( &inst_appl );
	  if( vi != problem.init_fluents().end() )
	    return( *new Value( (*vi).second ) );
	  else
	    throw Exception( "value of static function application undefined" );
	}
      else
	return( inst_appl );
    }
  else
    return( *this );
}

bool
Application::operator==( const Expression &expr ) const
{
  return( false );
}

void
Application::print( std::ostream& os, const FunctionTable& functions,
		    const TermTable& terms ) const
{
  os << '(';
  functions.print_function( os, function() );
  for( TermList::const_iterator ti = terms_.begin(); ti != terms_.end(); ++ti )
    {
      os << ' ';
      terms.print_term( os, *ti );
    }
  os << ')';
}

void
Application::printXML( std::ostream& os, const FunctionTable& functions,
		       const TermTable& terms ) const {
  os << "<application name=\"";
  functions.print_function( os, function() );
  os << "\">";
  for( TermList::const_iterator ti = terms_.begin(); ti != terms_.end(); ++ti )
    {
      os << "<term>";
      terms.print_term( os, *ti );
      os << "</term>";
    }
  os << "</application>";
}


/*******************************************************************************
 *
 * addition
 *
 ******************************************************************************/

Addition::Addition( const Expression& term1, const Expression& term2 )
  : term1_(&term1), term2_(&term2)
{
  Expression::register_use( term1_ );
  Expression::register_use( term2_ );
}

Addition::~Addition()
{
  Expression::unregister_use( term1_ );
  Expression::unregister_use( term2_ );
}

Rational
Addition::value( const ValueMap& values ) const
{
  return( term1().value( values ) + term2().value( values ) );
}

const Expression& 
Addition::instantiation( const SubstitutionMap& subst, const problem_t& problem ) const
{
  const Expression& inst_term1 = term1().instantiation( subst, problem );
  const Expression& inst_term2 = term2().instantiation( subst, problem );
  const Value* v1 = dynamic_cast<const Value*>( &inst_term1 );
  if( v1 != NULL )
    {
      const Value* v2 = dynamic_cast<const Value*>( &inst_term2 );
      if( v2 != NULL )
	{
	  const Value* value = new Value( v1->value() + v2->value() );
	  Expression::register_use( v1 );
	  Expression::register_use( v2 );
	  Expression::unregister_use( v1 );
	  Expression::unregister_use( v2 );
	  return( *value );
	}
    }
  return( *new Addition( inst_term1, inst_term2 ) );
}

bool
Addition::operator==( const Expression &expr ) const
{
  const Addition *aexpr = dynamic_cast<const Addition*>( &expr );
  return( (aexpr != NULL) && (*term1_ == *aexpr->term1_) && (*term2_ == *aexpr->term2_) );
}

void
Addition::print( std::ostream& os, const FunctionTable& functions,
		 const TermTable& terms ) const
{
  os << "(+ ";
  term1().print( os, functions, terms );
  os << ' ';
  term2().print( os, functions, terms );
  os << ")";
}


/*******************************************************************************
 *
 * subtraction
 *
 ******************************************************************************/

Subtraction::Subtraction( const Expression& term1, const Expression& term2 )
  : term1_(&term1), term2_(&term2)
{
  Expression::register_use( term1_ );
  Expression::register_use( term2_ );
}

Subtraction::~Subtraction()
{
  Expression::unregister_use( term1_ );
  Expression::unregister_use( term2_ );
}

Rational
Subtraction::value( const ValueMap& values ) const
{
  return( term1().value( values ) - term2().value( values ) );
}

const Expression& 
Subtraction::instantiation( const SubstitutionMap& subst, const problem_t& problem ) const
{
  const Expression& inst_term1 = term1().instantiation( subst, problem );
  const Expression& inst_term2 = term2().instantiation( subst, problem );
  const Value* v1 = dynamic_cast<const Value*>( &inst_term1 );
  if( v1 != NULL )
    {
      const Value* v2 = dynamic_cast<const Value*>( &inst_term2 );
      if( v2 != NULL )
	{
	  const Value* value = new Value( v1->value() - v2->value() );
	  Expression::register_use( v1 );
	  Expression::register_use( v2 );
	  Expression::unregister_use( v1 );
	  Expression::unregister_use( v2 );
	  return( *value );
	}
    }
  return( *new Subtraction( inst_term1, inst_term2 ) );
}

bool
Subtraction::operator==( const Expression &expr ) const
{
  const Subtraction *sexpr = dynamic_cast<const Subtraction*>( &expr );
  return( (sexpr != NULL) && (*term1_ == *sexpr->term1_) && (*term2_ == *sexpr->term2_) );
}

void
Subtraction::print( std::ostream& os, const FunctionTable& functions,
		    const TermTable& terms ) const
{
  os << "(- ";
  term1().print( os, functions, terms );
  os << ' ';
  term2().print( os, functions, terms );
  os << ")";
}


/*******************************************************************************
 *
 * multiplication
 *
 ******************************************************************************/

Multiplication::Multiplication( const Expression& factor1, const Expression& factor2 )
  : factor1_(&factor1), factor2_(&factor2)
{
  Expression::register_use( factor1_ );
  Expression::register_use( factor2_ );
}

Multiplication::~Multiplication()
{
  Expression::unregister_use( factor1_ );
  Expression::unregister_use( factor2_ );
}

Rational
Multiplication::value( const ValueMap& values ) const
{
  return( factor1().value( values ) * factor2().value( values ) );
}

const Expression& 
Multiplication::instantiation( const SubstitutionMap& subst, const problem_t& problem ) const
{
  const Expression& inst_factor1 = factor1().instantiation( subst, problem );
  const Expression& inst_factor2 = factor2().instantiation( subst, problem );
  const Value* v1 = dynamic_cast<const Value*>( &inst_factor1 );
  if( v1 != NULL )
    {
      const Value* v2 = dynamic_cast<const Value*>( &inst_factor2 );
      if( v2 != NULL )
	{
	  const Value* value = new Value( v1->value() * v2->value() );
	  Expression::register_use( v1 );
	  Expression::register_use( v2 );
	  Expression::unregister_use( v1 );
	  Expression::unregister_use( v2 );
	  return( *value );
	}
    }
  return( *new Multiplication( inst_factor1, inst_factor2 ) );
}

bool
Multiplication::operator==( const Expression &expr ) const
{
  const Multiplication *mexpr = dynamic_cast<const Multiplication*>( &expr );
  return( (mexpr != NULL) && (*factor1_ == *mexpr->factor1_) && (*factor2_ == *mexpr->factor2_) );
}

void
Multiplication::print( std::ostream& os, const FunctionTable& functions,
		       const TermTable& terms ) const
{
  os << "(* ";
  factor1().print( os, functions, terms );
  os << ' ';
  factor2().print( os, functions, terms );
  os << ")";
}


/*******************************************************************************
 *
 * division
 *
 ******************************************************************************/

Division::Division( const Expression& factor1, const Expression& factor2 )
  : factor1_(&factor1), factor2_(&factor2)
{
  Expression::register_use( factor1_ );
  Expression::register_use( factor2_ );
}

Division::~Division()
{
  Expression::unregister_use( factor1_ );
  Expression::unregister_use( factor2_ );
}

Rational
Division::value( const ValueMap& values ) const
{
  return( factor1().value( values ) / factor2().value( values ) );
}

const Expression& 
Division::instantiation( const SubstitutionMap& subst, const problem_t& problem ) const
{
  const Expression& inst_factor1 = factor1_->instantiation( subst, problem );
  const Expression& inst_factor2 = factor2_->instantiation( subst, problem );
  const Value* v1 = dynamic_cast<const Value*>( &inst_factor1 );
  if( v1 != NULL )
    {
      const Value* v2 = dynamic_cast<const Value*>( &inst_factor2 );
      if( v2 != NULL )
	{
	  const Value* value = new Value( v1->value() / v2->value() );
	  Expression::register_use( v1 );
	  Expression::register_use( v2 );
	  Expression::unregister_use( v1 );
	  Expression::unregister_use( v2 );
	  return( *value );
	}
    }
  return( *new Division( inst_factor1, inst_factor2 ) );
}

bool
Division::operator==( const Expression &expr ) const
{
  const Division *dexpr = dynamic_cast<const Division*>( &expr );
  return( (dexpr != NULL) && (*factor1_ == *dexpr->factor1_) && (*factor2_ == *dexpr->factor2_) );
}

void
Division::print( std::ostream& os, const FunctionTable& functions,
		 const TermTable& terms ) const
{
  os << "(/ ";
  factor1().print( os, functions, terms );
  os << ' ';
  factor2().print( os, functions, terms );
  os << ")";
}
