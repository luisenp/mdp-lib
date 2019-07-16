#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include "functions.h"
#include "terms.h"
#include "rational.h"
#include <assert.h>
#include <iostream>
#include <set>
#include <unordered_map>

class problem_t;
class Application;


/*******************************************************************************
 *
 * expression
 *
 ******************************************************************************/

class ValueMap : public std::unordered_map<const Application*,Rational> { };

class Expression
{
  mutable size_t ref_count_;

protected:
  Expression() : ref_count_(0) { }

public:
  virtual ~Expression()
    {
      assert( ref_count_ == 0 );
    }

  static void register_use( const Expression* e )
    {
#ifdef MEM_DEBUG
      if(e) std::cerr << "<exp>: inc-ref-count " << e << " = " << e->ref_count_+1 << std::endl;
#endif
      if( e ) ++e->ref_count_;
    }

  static void unregister_use( const Expression* e )
    {
#ifdef MEM_DEBUG
      if(e) std::cerr << "<exp>: dec-ref-count " << e << " = " << e->ref_count_-1 << std::endl;
#endif
      if( e && (--e->ref_count_ == 0) ) delete e;
    }

  virtual Rational value( const ValueMap& values ) const = 0;
  virtual const Expression& instantiation( const SubstitutionMap& subst,
					   const problem_t& problem ) const = 0;
  virtual bool operator==( const Expression &expr ) const = 0;
  virtual void print( std::ostream& os, const FunctionTable& functions,
		      const TermTable& terms ) const = 0;
};


/*******************************************************************************
 *
 * value
 *
 ******************************************************************************/

class Value : public Expression
{
  Rational value_;

public:
  Value( const Rational& value ) { value_ = value; }
  const Rational& value( void ) const { return( value_ ); }
  virtual Rational value( const ValueMap& values ) const;
  virtual const Value& instantiation( const SubstitutionMap& subst,
				      const problem_t& problem ) const;
  virtual bool operator==( const Expression &expr ) const;
  virtual void print( std::ostream& os, const FunctionTable& functions,
		      const TermTable& terms ) const;
};


/*******************************************************************************
 *
 * application
 *
 ******************************************************************************/

class Application : public Expression
{
  class ApplicationLess : public std::binary_function<const Application*,const Application*,bool>
  {
  public:
    bool operator()( const Application* a1, const Application* a2 ) const;
  };

  class ApplicationTable : public std::set<const Application*,ApplicationLess> { };

  static ApplicationTable applications;
  Function function_;
  TermList terms_;
  Application( Function function ) : function_(function) { }
  void add_term( Term term ) { terms_.push_back( term ); }

public:
  virtual ~Application();
  static const Application& make_application( Function function,
					      const TermList& terms );
  Function function( void ) const { return( function_ ); }
  size_t arity( void ) const { return( terms_.size() ); }
  Term term( size_t i ) const { return( terms_[i] ); }
  virtual Rational value( const ValueMap& values ) const;
  const Application& substitution( const SubstitutionMap& subst ) const;
  virtual const Expression& instantiation( const SubstitutionMap& subst,
					   const problem_t& problem ) const;
  virtual bool operator==( const Expression &expr ) const;
  virtual void print( std::ostream& os, const FunctionTable& functions,
		      const TermTable& terms ) const;
  void printXML( std::ostream& os, const FunctionTable& functions,
		 const TermTable& terms ) const;
};

/*******************************************************************************
 *
 * addition
 *
 ******************************************************************************/

class Addition : public Expression
{
  const Expression* term1_;
  const Expression* term2_;

public:
  Addition( const Expression& term1, const Expression& term2 );
  virtual ~Addition();
  const Expression& term1( void ) const { return( *term1_ ); }
  const Expression& term2( void ) const { return( *term2_ ); }
  virtual Rational value( const ValueMap& values ) const;
  virtual const Expression& instantiation( const SubstitutionMap& subst,
					   const problem_t& problem ) const;
  virtual bool operator==( const Expression &expr ) const;
  virtual void print( std::ostream& os, const FunctionTable& functions,
		      const TermTable& terms ) const;
};


/*******************************************************************************
 *
 * subtraction
 *
 ******************************************************************************/

class Subtraction : public Expression
{
  const Expression* term1_;
  const Expression* term2_;

public:
  Subtraction( const Expression& term1, const Expression& term2 );
  virtual ~Subtraction();
  const Expression& term1( void ) const { return( *term1_ ); }
  const Expression& term2( void ) const { return( *term2_ ); }
  virtual Rational value( const ValueMap& values ) const;
  virtual const Expression& instantiation( const SubstitutionMap& subst,
					   const problem_t& problem ) const;
  virtual bool operator==( const Expression &expr ) const;
  virtual void print( std::ostream& os, const FunctionTable& functions,
		      const TermTable& terms ) const;
};


/*******************************************************************************
 *
 * multiplication
 *
 ******************************************************************************/

class Multiplication : public Expression
{
  const Expression* factor1_;
  const Expression* factor2_;

public:
  Multiplication( const Expression& factor1, const Expression& factor2 );
  virtual ~Multiplication();
  const Expression& factor1( void ) const { return( *factor1_ ); }
  const Expression& factor2( void ) const { return( *factor2_ ); }
  virtual Rational value( const ValueMap& values ) const;
  virtual const Expression& instantiation( const SubstitutionMap& subst,
					   const problem_t& problem ) const;
  virtual bool operator==( const Expression &expr ) const;
  virtual void print( std::ostream& os, const FunctionTable& functions,
		      const TermTable& terms ) const;
};


/*******************************************************************************
 *
 * division
 *
 ******************************************************************************/

class Division : public Expression
{
  const Expression* factor1_;
  const Expression* factor2_;

public:
  Division( const Expression& factor1, const Expression& factor2 );
  virtual ~Division();
  const Expression& factor1( void ) const { return( *factor1_ ); }
  const Expression& factor2( void ) const { return( *factor2_ ); }
  virtual Rational value( const ValueMap& values ) const;
  virtual const Expression& instantiation( const SubstitutionMap& subst,
					   const problem_t& problem ) const;
  virtual bool operator==( const Expression &expr ) const;
  virtual void print( std::ostream& os, const FunctionTable& functions,
		      const TermTable& terms ) const;
};

#endif // EXPRESSIONS_H
