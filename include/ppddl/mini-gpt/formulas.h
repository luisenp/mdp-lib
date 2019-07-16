#ifndef FORMULAS_H
#define FORMULAS_H

#include "global.h"
#include "predicates.h"
#include "terms.h"
#include <iostream>
#include <set>
#include <vector>
#include <list>
#include <unordered_set>

#ifdef TRUE
#undef TRUE
#endif
#ifdef FALSE
#undef FALSE
#endif

class atomList_t;
class atomListList_t;
class state_t;
class problem_t;

class FunctionTable;
class Expression;
class ValueMap;
class Problem;
class FormulaList;

class Atom;
class AtomSet : public std::unordered_set<const Atom*> { };
class Disjunction;


/*******************************************************************************
 *
 * state formula
 *
 ******************************************************************************/

class StateFormula
{
private:
  mutable size_t ref_count_;

protected:
  StateFormula() : ref_count_(0) { }

public:
  static const StateFormula& TRUE;
  static const StateFormula& FALSE;

  static void register_use( const StateFormula* f )
    {
#ifdef MEM_DEBUG
      if(f) std::cerr << "<for>: inc-ref-count " << f << " = " << f->ref_count_+1 << std::endl;
#endif
      if( f ) ++f->ref_count_;
    }
  static void unregister_use( const StateFormula* f )
    {
#ifdef MEM_DEBUG
      if(f) std::cerr << "<for>: dec-ref-count " << f << " = " << f->ref_count_-1 << std::endl;
#endif
      if( f && (--f->ref_count_ == 0) ) delete f;
    }

  virtual ~StateFormula() { }
  bool tautology( void ) const { return this == &TRUE; }
  bool contradiction( void ) const { return this == &FALSE; }
  virtual bool holds( const AtomSet& atoms, const ValueMap& values ) const = 0;
  virtual bool holds( const state_t& state ) const = 0;
  virtual const StateFormula& flatten( bool negated = false ) const = 0;
  virtual void translate( atomList_t &alist ) const = 0;
  virtual void translate( atomListList_t &alist ) const = 0;
  virtual const StateFormula& instantiation( const SubstitutionMap& subst,
					     const problem_t& problem ) const = 0;
  virtual bool operator==( const StateFormula &formula ) const = 0;
  virtual void print( std::ostream& os, const PredicateTable& predicates,
		      const FunctionTable& functions,
		      const TermTable& terms) const = 0;

  virtual bool match( const StateFormula &formula, VarSubstMap &subst ) const = 0;
  virtual void free_vars( VariableList &context, VariableList &vars ) const = 0;
  virtual bool analyze( PredicateTable &predicates, TermTable &terms,
			std::map<const StateFormula*,const Atom*> &hash ) const = 0;
  virtual const StateFormula& rewrite( std::map<const StateFormula*,const Atom*> &hash ) const = 0;
  virtual void generate_atoms( void ) const = 0;
};

class FormulaList : public std::vector<const StateFormula*> { };


/*******************************************************************************
 *
 * constant
 *
 ******************************************************************************/

class Constant : public StateFormula
{
private:
  static const Constant TRUE_;
  static const Constant FALSE_;

  bool value_;
  Constant( bool value );

  friend class StateFormula;

public:
  virtual bool holds( const AtomSet& atoms, const ValueMap& values ) const;
  virtual bool holds( const state_t& state ) const;
  virtual const StateFormula& flatten( bool negated = false ) const;
  virtual void translate( atomList_t &alist ) const;
  virtual void translate( atomListList_t &alist ) const;
  virtual const Constant& instantiation( const SubstitutionMap& subst,
					 const problem_t& problem ) const;
  virtual bool operator==( const StateFormula &formula ) const;
  virtual void print( std::ostream& os, const PredicateTable& predicates,
		      const FunctionTable& functions,
		      const TermTable& terms ) const;


  virtual bool match( const StateFormula &formula, VarSubstMap &subst ) const;
  virtual void free_vars( VariableList &context, VariableList &vars ) const;
  virtual bool analyze( PredicateTable &predicates, TermTable &terms,
			std::map<const StateFormula*,const Atom*> &hash ) const;
  virtual const StateFormula& rewrite( std::map<const StateFormula*,const Atom*> &hash ) const;
  virtual void generate_atoms( void ) const;
};


/*******************************************************************************
 *
 * atom
 *
 ******************************************************************************/

class Atom : public StateFormula
{
private:
  // Less-than comparison function object for atoms.
  class AtomLess : public std::binary_function<const Atom*, const Atom*, bool>
  {
  public:
    bool operator()( const Atom* a1, const Atom* a2 ) const;
  };

  class AtomTable : public std::set<const Atom*,AtomLess> { };
  static AtomTable atoms;

  Predicate predicate_;
  TermList terms_;

  Atom( Predicate predicate );
  void add_term( Term term ) { terms_.push_back( term ); }

public:
  static const Atom& make_atom( Predicate predicate, const TermList& terms );
  virtual ~Atom();

  Predicate predicate( void ) const { return( predicate_ ); }
  size_t arity( void ) const { return( terms_.size() ); }
  Term term( size_t i ) const { return( terms_[i] ); }
  virtual bool holds( const AtomSet& atoms, const ValueMap& values ) const;
  virtual bool holds( const state_t& state ) const;
  virtual const StateFormula& flatten( bool negated = false ) const;
  virtual void translate( atomList_t &alist ) const;
  virtual void translate( atomListList_t &alist ) const;
  const Atom& substitution( const SubstitutionMap& subst ) const;
  virtual const StateFormula& instantiation( const SubstitutionMap& subst,
					     const problem_t& problem ) const;
  virtual bool operator==( const StateFormula &formula ) const;
  virtual void print( std::ostream& os, const PredicateTable& predicates,
		      const FunctionTable& functions,
		      const TermTable& terms ) const;
  void printXML( std::ostream& os, const PredicateTable& predicates,
		 const FunctionTable& functions,
		 const TermTable& terms ) const;


  virtual bool match( const StateFormula &formula, VarSubstMap &subst ) const;
  virtual void free_vars( VariableList &context, VariableList &vars ) const;
  virtual bool analyze( PredicateTable &predicates, TermTable &terms,
			std::map<const StateFormula*,const Atom*> &hash ) const;
  virtual const StateFormula& rewrite( std::map<const StateFormula*,const Atom*> &hash ) const;
  virtual void generate_atoms( void ) const;
};


/*******************************************************************************
 *
 * equality
 *
 ******************************************************************************/

class Equality : public StateFormula
{
private:
  Term term1_;
  Term term2_;

public:
  Equality( Term term1, Term term2 );
  virtual ~Equality();

  Term term1( void ) const { return( term1_ ); }
  Term term2( void ) const { return( term2_ ); }
  virtual bool holds( const AtomSet& atoms, const ValueMap& values ) const;
  virtual bool holds( const state_t& state ) const;
  virtual const StateFormula& flatten( bool negated = false ) const;
  virtual void translate( atomList_t &alist ) const;
  virtual void translate( atomListList_t &alist ) const;
  virtual const StateFormula& instantiation( const SubstitutionMap& subst,
					     const problem_t& problem ) const;
  virtual bool operator==( const StateFormula &formula ) const;
  virtual void print( std::ostream& os, const PredicateTable& predicates,
		      const FunctionTable& functions,
		      const TermTable& terms ) const;

  virtual bool match( const StateFormula &formula, VarSubstMap &subst ) const;
  virtual void free_vars( VariableList &context, VariableList &vars ) const;
  virtual bool analyze( PredicateTable &predicates, TermTable &terms,
			std::map<const StateFormula*,const Atom*> &hash ) const;
  virtual const StateFormula& rewrite( std::map<const StateFormula*,const Atom*> &hash ) const;
  virtual void generate_atoms( void ) const;
};


/*******************************************************************************
 *
 * comparison
 *
 ******************************************************************************/

class Comparison : public StateFormula
{
public:
  typedef enum { LT_CMP, LE_CMP, EQ_CMP, GE_CMP, GT_CMP } CmpPredicate;

private:
  CmpPredicate predicate_;
  const Expression* expr1_;
  const Expression* expr2_;

public:
  Comparison( CmpPredicate predicate, const Expression& expr1, const Expression& expr2 );
  virtual ~Comparison();

  CmpPredicate predicate( void ) const { return( predicate_ ); }
  const Expression& expr1( void ) const { return( *expr1_ ); }
  const Expression& expr2( void ) const { return( *expr2_ ); }
  virtual bool holds( const AtomSet& atoms, const ValueMap& values ) const;
  virtual bool holds( const state_t& state ) const;
  virtual const StateFormula& flatten( bool negated = false ) const;
  virtual void translate( atomList_t &alist ) const;
  virtual void translate( atomListList_t &alist ) const;
  virtual const StateFormula& instantiation( const SubstitutionMap& subst,
					     const problem_t& problem ) const;
  virtual bool operator==( const StateFormula &formula ) const;
  virtual void print( std::ostream& os, const PredicateTable& predicates,
		      const FunctionTable& functions,
		      const TermTable& terms ) const;

  virtual bool match( const StateFormula &formula, VarSubstMap &subst ) const;
  virtual void free_vars( VariableList &context, VariableList &vars ) const;
  virtual bool analyze( PredicateTable &predicates, TermTable &terms,
			std::map<const StateFormula*,const Atom*> &hash ) const;
  virtual const StateFormula& rewrite( std::map<const StateFormula*,const Atom*> &hash ) const;
  virtual void generate_atoms( void ) const;
};


/*******************************************************************************
 *
 * negation
 *
 ******************************************************************************/

class Negation : public StateFormula
{
private:
  const StateFormula* negand_;
  Negation( const StateFormula& negand );

public:
  static const StateFormula& make_negation( const StateFormula& formula );
  virtual ~Negation();

  const StateFormula& negand( void ) const { return( *negand_ ); }
  virtual bool holds( const AtomSet& atoms, const ValueMap& values ) const;
  virtual bool holds( const state_t& state ) const;
  virtual const StateFormula& flatten( bool negated = false ) const;
  virtual void translate( atomList_t &alist ) const;
  virtual void translate( atomListList_t &alist ) const;
  virtual const StateFormula& instantiation( const SubstitutionMap& subst,
					     const problem_t& problem ) const;
  virtual bool operator==( const StateFormula &formula ) const;
  virtual void print( std::ostream& os, const PredicateTable& predicates,
		      const FunctionTable& functions,
		      const TermTable& terms ) const;

  virtual bool match( const StateFormula &formula, VarSubstMap &subst ) const;
  virtual void free_vars( VariableList &context, VariableList &vars ) const;
  virtual bool analyze( PredicateTable &predicates, TermTable &terms,
			std::map<const StateFormula*,const Atom*> &hash ) const;
  virtual const StateFormula& rewrite( std::map<const StateFormula*,const Atom*> &hash ) const;
  virtual void generate_atoms( void ) const;
};


/*******************************************************************************
 *
 * conjunction
 *
 ******************************************************************************/

class Conjunction : public StateFormula
{
private:
  FormulaList conjuncts_;

public:
  Conjunction();
  virtual ~Conjunction();

  static void cross_products( std::list<const Disjunction*>::const_iterator it,
			      std::list<const Disjunction*>::const_iterator end,
			      std::list<const StateFormula*> &conj,
			      std::list<const Conjunction*> &xproduct );

  const Conjunction& merge( const Conjunction &conjunction ) const;
  void add_conjunct( const StateFormula& conjunct );
  size_t size( void ) const { return( conjuncts_.size() ); }
  const StateFormula& conjunct( size_t i ) const { return( *conjuncts_[i] ); }
  virtual bool holds( const AtomSet& atoms, const ValueMap& values ) const;
  virtual bool holds( const state_t& state ) const;
  virtual const StateFormula& flatten( bool negated = false ) const;
  virtual void translate( atomList_t &alist ) const;
  virtual void translate( atomListList_t &alist ) const;
  virtual const StateFormula& instantiation( const SubstitutionMap& subst,
					     const problem_t& problem ) const;
  virtual bool operator==( const StateFormula &formula ) const;
  virtual void print( std::ostream& os, const PredicateTable& predicates,
		      const FunctionTable& functions,
		      const TermTable& terms ) const;

  virtual bool match( const StateFormula &formula, VarSubstMap &subst ) const;
  virtual void free_vars( VariableList &context, VariableList &vars ) const;
  virtual bool analyze( PredicateTable &predicates, TermTable &terms,
			std::map<const StateFormula*,const Atom*> &hash ) const;
  virtual const StateFormula& rewrite( std::map<const StateFormula*,const Atom*> &hash ) const;
  virtual void generate_atoms( void ) const;
};


/*******************************************************************************
 *
 * disjunction
 *
 ******************************************************************************/

class Disjunction : public StateFormula
{
private:
  FormulaList disjuncts_;

public:
  Disjunction();
  virtual ~Disjunction();

  static void cross_products( std::list<const Conjunction*>::const_iterator it,
			      std::list<const Conjunction*>::const_iterator end,
			      std::list<const StateFormula*> &conj,
			      std::list<const Disjunction*> &xproduct );

  const Disjunction& merge( const Disjunction &disjunction ) const;
  void add_disjunct( const StateFormula& disjunct );
  size_t size( void ) const { return( disjuncts_.size() ); }
  const StateFormula& disjunct( size_t i ) const { return( *disjuncts_[i] ); }
  virtual bool holds( const AtomSet& atoms, const ValueMap& values ) const;
  virtual bool holds( const state_t& state ) const;
  virtual const StateFormula& flatten( bool negated = false ) const;
  virtual void translate( atomList_t &alist ) const;
  virtual void translate( atomListList_t &alist ) const;
  virtual const StateFormula& instantiation( const SubstitutionMap& subst,
					     const problem_t& problem ) const;
  virtual bool operator==( const StateFormula &formula ) const;
  virtual void print( std::ostream& os, const PredicateTable& predicates,
		      const FunctionTable& functions,
		      const TermTable& terms ) const;

  virtual bool match( const StateFormula &formula, VarSubstMap &subst ) const;
  virtual void free_vars( VariableList &context, VariableList &vars ) const;
  virtual bool analyze( PredicateTable &predicates, TermTable &terms,
			std::map<const StateFormula*,const Atom*> &hash ) const;
  virtual const StateFormula& rewrite( std::map<const StateFormula*,const Atom*> &hash ) const;
  virtual void generate_atoms( void ) const;
};


/*******************************************************************************
 *
 * exists
 *
 ******************************************************************************/

class Exists : public StateFormula
{
private:
  VariableList parameters_;
  const StateFormula* body_;

public:
  Exists();
  virtual ~Exists();

  void add_parameter( Variable parameter ) { parameters_.push_back( parameter ); }
  void set_body( const StateFormula& body );
  size_t arity( void ) const { return( parameters_.size() ); }
  Variable parameter( size_t i ) const { return( parameters_[i] ); }
  const StateFormula& body( void ) const { return( *body_ ); }
  virtual bool holds( const AtomSet& atoms, const ValueMap& values ) const;
  virtual bool holds( const state_t& state ) const;
  virtual const StateFormula& flatten( bool negated = false ) const;
  virtual void translate( atomList_t &alist ) const;
  virtual void translate( atomListList_t &alist ) const;
  virtual const StateFormula& instantiation( const SubstitutionMap& subst,
					     const problem_t& problem ) const;
  virtual bool operator==( const StateFormula &formula ) const;
  virtual void print( std::ostream& os, const PredicateTable& predicates,
		      const FunctionTable& functions,
		      const TermTable& terms ) const;

  virtual bool match( const StateFormula &formula, VarSubstMap &subst ) const;
  virtual void free_vars( VariableList &context, VariableList &vars ) const;
  virtual bool analyze( PredicateTable &predicates, TermTable &terms,
			std::map<const StateFormula*,const Atom*> &hash ) const;
  virtual const StateFormula& rewrite( std::map<const StateFormula*,const Atom*> &hash ) const;
  virtual void generate_atoms( void ) const;
};


/*******************************************************************************
 *
 * forall
 *
 ******************************************************************************/

class Forall : public StateFormula
{
private:
  VariableList parameters_;
  const StateFormula* body_;

public:
  Forall();
  virtual ~Forall();

  void add_parameter( Variable parameter ) { parameters_.push_back( parameter ); }
  void set_body( const StateFormula& body );
  size_t arity( void ) const { return( parameters_.size() ); }
  Variable parameter( size_t i ) const { return( parameters_[i] ); }
  const StateFormula& body( void ) const { return( *body_ ); }
  virtual bool holds( const AtomSet& atoms, const ValueMap& values ) const;
  virtual bool holds( const state_t& state ) const;
  virtual const StateFormula& flatten( bool negated = false ) const;
  virtual void translate( atomList_t &alist ) const;
  virtual void translate( atomListList_t &alist ) const;
  virtual const StateFormula& instantiation( const SubstitutionMap& subst,
					     const problem_t& problem ) const;
  virtual bool operator==( const StateFormula &formula ) const;
  virtual void print( std::ostream& os, const PredicateTable& predicates,
		      const FunctionTable& functions,
		      const TermTable& terms ) const;

  virtual bool match( const StateFormula &formula, VarSubstMap &subst ) const;
  virtual void free_vars( VariableList &context, VariableList &vars ) const;
  virtual bool analyze( PredicateTable &predicates, TermTable &terms,
			std::map<const StateFormula*,const Atom*> &hash ) const;
  virtual const StateFormula& rewrite( std::map<const StateFormula*,const Atom*> &hash ) const;
  virtual void generate_atoms( void ) const;
};

class AtomList : public std::vector<const Atom*> { };

#endif // FORMULAS_H
