#ifndef EFFECTS_H
#define EFFECTS_H

#include "global.h"
#include "terms.h"
#include <assert.h>
#include <iostream>
#include <vector>

class Rational;
class PredicateTable;
class FunctionTable;
class ValueMap;
class Expression;
class Application;
class StateFormula;
class Atom;
class AtomSet;
class AtomList;

class stripsEffect_t;
class conditionalEffectList_t;
class probabilisticEffectList_t;
class state_t;
class stateProbList_t;
class problem_t;

class Assignment
{
public:
  enum { ASSIGN_OP, SCALE_UP_OP, SCALE_DOWN_OP, INCREASE_OP, DECREASE_OP };

  Assignment( unsigned oper, const Application& application, const Expression& expr );
  ~Assignment();

  const Application& application( void ) const { return( *application_ ); }
  void affect( ValueMap& values ) const;
  void affect( state_t& state ) const;
  const Assignment& instantiation( const SubstitutionMap& subst,
				   const problem_t& problem ) const;
  bool operator==( const Assignment &assig ) const;
  void print( std::ostream& os, const FunctionTable& functions,
	      const TermTable& terms ) const;

private:
  unsigned operator_;
  const Application* application_;
  const Expression* expr_;
};

class AssignmentList : public std::vector<const Assignment*> { };

class Effect
{
  mutable size_t ref_count_;

protected:
  Effect() : ref_count_(0) { }

public:
  virtual ~Effect()
    {
      assert( ref_count_ == 0 );
    }

  static void register_use( const Effect* e )
    {
#ifdef MEM_DEBUG
      if(e) std::cerr << "<eff>: inc-ref-count " << e << " = " << e->ref_count_+1 << std::endl;
#endif
      if( e ) e->ref_count_++;
    }
  static void unregister_use( const Effect* e )
    {
#ifdef MEM_DEBUG
      if(e) std::cerr << "<eff>: dec-ref-count " << e << " = " << e->ref_count_-1 << std::endl;
#endif
      if( e && (--e->ref_count_ == 0) ) delete e;
    }

  virtual const Effect& flatten( void ) const = 0;
  virtual void state_change( AtomList& adds, AtomList& deletes,
			     AssignmentList& assignments,
			     const state_t& state ) const = 0;
  virtual const stateProbList_t& expand( const stateProbList_t &state_list ) const = 0;
  virtual void translate( stripsEffect_t &s_effect,
			  conditionalEffectList_t &c_effect ) const = 0;
  virtual const Effect& instantiation( const SubstitutionMap& subst,
				       const problem_t& problem ) const = 0;
  virtual bool operator==( const Effect& eff ) const = 0;
  virtual void print( std::ostream& os, const PredicateTable& predicates,
		      const FunctionTable& functions,
		      const TermTable& terms ) const = 0;

  virtual void analyze( PredicateTable &predicates, TermTable &terms,
			std::map<const StateFormula*,const Atom*> &hash ) const = 0;
  virtual const Effect& rewrite( std::map<const StateFormula*,const Atom*> &hash ) const = 0;
};

class EffectList : public std::vector<const Effect*> { };

class AddEffect : public Effect
{
  const Atom* atom_;

public:
  AddEffect( const Atom& atom );
  virtual ~AddEffect();

  const Atom& atom( void ) const { return( *atom_ ); }

  virtual const Effect& flatten( void ) const;
  virtual void state_change( AtomList& adds, AtomList& deletes,
			     AssignmentList& assignments,
			     const state_t& state ) const;
  virtual const stateProbList_t& expand( const stateProbList_t &state_list ) const;
  virtual void translate( stripsEffect_t &s_effect,
			  conditionalEffectList_t &c_effect ) const;
  virtual const Effect& instantiation( const SubstitutionMap& subst,
				       const problem_t& problem ) const;
  virtual bool operator==( const Effect& eff ) const;
  virtual void print( std::ostream& os, const PredicateTable& predicates,
		      const FunctionTable& functions,
		      const TermTable& terms ) const;

  virtual void analyze( PredicateTable &predicates, TermTable &terms,
			std::map<const StateFormula*,const Atom*> &hash ) const;
  virtual const Effect& rewrite( std::map<const StateFormula*,const Atom*> &hash ) const;
};


class DeleteEffect : public Effect
{
  const Atom* atom_;

public:
  DeleteEffect( const Atom& atom );
  virtual ~DeleteEffect();

  const Atom& atom( void ) const { return( *atom_ ); }

  virtual const Effect& flatten( void ) const;
  virtual void state_change( AtomList& adds, AtomList& deletes,
			     AssignmentList& assignments,
			     const state_t& state ) const;
  virtual const stateProbList_t& expand( const stateProbList_t &state_list ) const;
  virtual void translate( stripsEffect_t &s_effect,
			  conditionalEffectList_t &c_effect ) const;
  virtual const Effect& instantiation( const SubstitutionMap& subst,
				       const problem_t& problem ) const;
  virtual bool operator==( const Effect& eff ) const;
  virtual void print( std::ostream& os, const PredicateTable& predicates,
		      const FunctionTable& functions,
		      const TermTable& terms ) const;

  virtual void analyze( PredicateTable &predicates, TermTable &terms,
			std::map<const StateFormula*,const Atom*> &hash ) const;
  virtual const Effect& rewrite( std::map<const StateFormula*,const Atom*> &hash ) const;
};

class AssignmentEffect : public Effect
{
  const Assignment* assignment_;

public:
  AssignmentEffect( const Assignment& assignment );
  virtual ~AssignmentEffect();

  const Assignment& assignment( void ) const { return( *assignment_ ); }

  virtual const Effect& flatten( void ) const;
  virtual void state_change( AtomList& adds, AtomList& deletes,
			     AssignmentList& assignments,
			     const state_t& state ) const;
  virtual const stateProbList_t& expand( const stateProbList_t &state_list ) const;
  virtual void translate( stripsEffect_t &s_effect,
			  conditionalEffectList_t &c_effect ) const;
  virtual const Effect& instantiation( const SubstitutionMap& subst,
				       const problem_t& problem ) const;
  virtual bool operator==( const Effect& eff ) const;
  virtual void print( std::ostream& os, const PredicateTable& predicates,
		      const FunctionTable& functions,
		      const TermTable& terms ) const;

  virtual void analyze( PredicateTable &predicates, TermTable &terms,
			std::map<const StateFormula*,const Atom*> &hash ) const;
  virtual const Effect& rewrite( std::map<const StateFormula*,const Atom*> &hash ) const;
};

class ConjunctiveEffect : public Effect
{
  EffectList conjuncts_;

public:
  ConjunctiveEffect();
  virtual ~ConjunctiveEffect();

  void add_conjunct( const Effect& conjunct );
  size_t size( void ) const { return( conjuncts_.size() ); }
  const Effect& conjunct( size_t i ) const { return( *conjuncts_[i] ); }

  virtual const Effect& flatten( void ) const;
  virtual void state_change( AtomList& adds, AtomList& deletes,
			     AssignmentList& assignments,
			     const state_t& state ) const;
  virtual const stateProbList_t& expand( const stateProbList_t &state_list ) const;
  virtual void translate( stripsEffect_t &s_effect,
			  conditionalEffectList_t &c_effect ) const;
  virtual const Effect& instantiation( const SubstitutionMap& subst,
				       const problem_t& problem ) const;
  virtual bool operator==( const Effect& eff ) const;
  virtual void print( std::ostream& os, const PredicateTable& predicates,
		      const FunctionTable& functions,
		      const TermTable& terms ) const;

  virtual void analyze( PredicateTable &predicates, TermTable &terms,
			std::map<const StateFormula*,const Atom*> &hash ) const;
  virtual const Effect& rewrite( std::map<const StateFormula*,const Atom*> &hash ) const;
};

class ConditionalEffect : public Effect
{
  const StateFormula* condition_;
  const Effect* effect_;
  ConditionalEffect( const StateFormula& condition, const Effect& effect );
  static bool working_;

public:
  virtual ~ConditionalEffect();

  static const Effect& make( const StateFormula& condition,
			     const Effect& effect );

  const StateFormula& condition( void ) const { return( *condition_ ); }
  const Effect& effect( void ) const { return( *effect_ ); }

  virtual const Effect& flatten( void ) const;
  virtual void state_change( AtomList& adds, AtomList& deletes,
			     AssignmentList& assignments,
			     const state_t& state ) const;
  virtual const stateProbList_t& expand( const stateProbList_t &state_list ) const;
  virtual void translate( stripsEffect_t &s_effect,
			  conditionalEffectList_t &c_effect ) const;
  virtual const Effect& instantiation( const SubstitutionMap& subst,
				       const problem_t& problem ) const;
  virtual bool operator==( const Effect& eff ) const;
  virtual void print( std::ostream& os, const PredicateTable& predicates,
		      const FunctionTable& functions,
		      const TermTable& terms ) const;

  virtual void analyze( PredicateTable &predicates, TermTable &terms,
			std::map<const StateFormula*,const Atom*> &hash ) const;
  virtual const Effect& rewrite( std::map<const StateFormula*,const Atom*> &hash ) const;
};

class ProbabilisticEffect : public Effect
{
  std::vector<int> weights_;
  int weight_sum_;
  EffectList effects_;
  static bool working_;

public:
  ProbabilisticEffect();
  virtual ~ProbabilisticEffect();

  const ProbabilisticEffect& cross_product( const ProbabilisticEffect &effect ) const;
  bool add_outcome( const Rational& p, const Effect& effect );
  size_t size( void ) const { return( weights_.size() ); }
  Rational probability( size_t i ) const;
  const Effect& effect( size_t i ) const { return( *effects_[i] ); }

  virtual const Effect& flatten( void ) const;
  virtual void state_change( AtomList& adds, AtomList& deletes,
			     AssignmentList& assignments,
			     const state_t& state ) const;
  virtual const stateProbList_t& expand( const stateProbList_t &state_list ) const;
  void translate( probabilisticEffectList_t &plist ) const;
  virtual void translate( stripsEffect_t &s_effect,
			  conditionalEffectList_t &c_effect ) const;
  virtual const Effect& instantiation( const SubstitutionMap& subst,
				       const problem_t& problem ) const;
  virtual bool operator==( const Effect& eff ) const;
  virtual void print( std::ostream& os, const PredicateTable& predicates,
		      const FunctionTable& functions,
		      const TermTable& terms ) const;

  virtual void analyze( PredicateTable &predicates, TermTable &terms,
			std::map<const StateFormula*,const Atom*> &hash ) const;
  virtual const Effect& rewrite( std::map<const StateFormula*,const Atom*> &hash ) const;
};

class QuantifiedEffect : public Effect
{
  VariableList parameters_;
  const Effect* effect_;

public:
  QuantifiedEffect( const Effect& effect );
  virtual ~QuantifiedEffect();

  void add_parameter( Variable parameter ) { parameters_.push_back( parameter ); }
  size_t arity( void ) const { return( parameters_.size() ); }
  Variable parameter( size_t i ) const { return( parameters_[i] ); }
  const Effect& effect( void ) const { return( *effect_ ); }

  virtual const Effect& flatten( void ) const;
  virtual void state_change( AtomList& adds, AtomList& deletes,
			     AssignmentList& assignments,
			     const state_t& state ) const;
  virtual const stateProbList_t& expand( const stateProbList_t &state_list ) const;
  virtual void translate( stripsEffect_t &s_effect,
			  conditionalEffectList_t &c_effect ) const;
  virtual const Effect& instantiation( const SubstitutionMap& subst,
				       const problem_t& problem ) const;
  virtual bool operator==( const Effect& eff ) const;
  virtual void print( std::ostream& os, const PredicateTable& predicates,
		      const FunctionTable& functions,
		      const TermTable& terms ) const;

  virtual void analyze( PredicateTable &predicates, TermTable &terms,
			std::map<const StateFormula*,const Atom*> &hash ) const;
  virtual const Effect& rewrite( std::map<const StateFormula*,const Atom*> &hash ) const;
};

#endif /* EFFECTS_H */
