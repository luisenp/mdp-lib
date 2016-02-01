#ifndef ACTIONS_H
#define ACTIONS_H

#include "global.h"
#include "effects.h"
#include "formulas.h"
#include "rational.h"
#include "terms.h"
#include "utils.h"

#include <assert.h>
#include <iostream>
#include <list>
#include <map>
#include <string>

class PredicateTable;
class FunctionTable;
class ValueMap;
class StateFormula;
class AtomSet;
class Effect;

class action_t;
class state_t;
class stateProbList_t;
class problem_t;
class conditionalEffect_t;
class probabilisticEffect_t;

class Action;
class ActionList;


/*******************************************************************************
 *
 * action schema
 *
 ******************************************************************************/

class ActionSchema
{
  std::string name_;
  VariableList parameters_;
  const StateFormula* precondition_;
  const Effect* effect_;

public:
  ActionSchema( const std::string& name );
  ~ActionSchema();

  void add_parameter( Variable parameter ) { parameters_.push_back( parameter ); }
  void set_precondition( const StateFormula& precondition );
  void set_effect( const Effect& effect );
  const std::string& name( void ) const { return( name_ ); }
  size_t arity( void ) const { return( parameters_.size() ); }
  Variable parameter( size_t i ) const { return( parameters_[i] ); }
  const StateFormula& precondition( void ) const { return( *precondition_ ); }
  const Effect& effect( void ) const { return( *effect_ ); }
  void instantiations( ActionList& actions, const problem_t& problem ) const;
  const Action& instantiation( const SubstitutionMap& subst,
			       const problem_t& problem,
			       const StateFormula& precond ) const;
  void print( std::ostream& os, const PredicateTable& predicates,
	      const FunctionTable& functions, const TermTable& terms ) const;

  void analyze( PredicateTable &predicates, TermTable &terms,
		std::map<const StateFormula*,const Atom*> &hash ) const;
  const ActionSchema& rewrite( std::map<const StateFormula*,const Atom*> &hash ) const;
};

class ActionSchemaMap : public std::map<std::string, const ActionSchema*> { };


/*******************************************************************************
 *
 * action
 *
 ******************************************************************************/

class Action
{
  mutable size_t ref_count_;
  std::string name_;
  ObjectList arguments_;
  const StateFormula *precondition_;
  const Effect* effect_;

  Action() : ref_count_(0) { }

public:
  Action( const std::string& name );
  ~Action();

  static void register_use( const Action* a )
    {
#ifdef MEM_DEBUG
      if(a) std::cerr << "<ACT>: inc-ref-count " << a << " = " << a->ref_count_+1 << std::endl;
#endif
      if( a != NULL ) ++a->ref_count_;
    }
  static void unregister_use( const Action* a )
    {
#ifdef MEM_DEBUG
      if(a) std::cerr << "<ACT>: dec-ref-count " << a << " = " << a->ref_count_-1 << std::endl;
#endif
      if( a && (--a->ref_count_ == 0) ) delete a;
    }

  void add_argument( Object argument ) { arguments_.push_back( argument ); }
  void set_precondition( const StateFormula& precondition );
  void set_effect( const Effect& effect );
  const std::string& name( void ) const { return( name_ ); }
  size_t arity( void ) const { return( arguments_.size() ); }
  Object argument( size_t i ) const { return( arguments_[i] ); }
  const StateFormula& precondition( void ) const { return( *precondition_ ); }
  const Effect& effect( void ) const { return( *effect_ ); }

  const Action& flatten( const problem_t &problem ) const;
  const action_t& translate( const problem_t &problem ) const;

  bool enabled( const state_t& state ) const;
  void affect( state_t& state ) const;
  const stateProbList_t& expand( const state_t &state ) const;
  double cost( const state_t& state ) const { return( 1 ); }

  void print_full( std::ostream& os, const PredicateTable& predicates,
		   const FunctionTable& functions,
		   const TermTable& terms ) const;
  void print( std::ostream& os, const TermTable& terms ) const;
  void printXML( std::ostream& os, const TermTable& terms ) const;
};

class ActionList : public std::vector<const Action*> { };


/*******************************************************************************
 *
 * conditional effect list
 *
 ******************************************************************************/

class conditionalEffectList_t
{
  size_t size_;
  const conditionalEffect_t **data_, **data_ptr_;

public:
  conditionalEffectList_t() : size_(0), data_(0), data_ptr_(0) { }
  ~conditionalEffectList_t() { if( data_ ) free( data_ ); }

  size_t size( void ) const { return( data_ptr_ - data_ ); }
  const conditionalEffect_t& effect( size_t i ) const { return( *data_[i] ); }
  bool find( const conditionalEffect_t &eff ) const;
  void insert( const conditionalEffect_t *effect );
  bool affect( state_t &resulting_state,
               state_t &original_state,
               bool nprec = false ) const;
  void clear( void ) { data_ptr_ = data_; }
  void print( std::ostream &os ) const;
  bool operator==( const conditionalEffectList_t &clist ) const;
  conditionalEffectList_t& operator=( const conditionalEffectList_t &effect );
};


/*******************************************************************************
 *
 * probabilistic effect list
 *
 ******************************************************************************/

class probabilisticEffectList_t
{
  size_t size_;
  const probabilisticEffect_t **data_, **data_ptr_;

public:
  probabilisticEffectList_t() : size_(0), data_(0), data_ptr_(0) { }
  ~probabilisticEffectList_t() { if( data_ ) free( data_ ); }

  size_t size( void ) const { return( data_ptr_ - data_ ); }
  const probabilisticEffect_t& effect( size_t i ) const { return( *data_[i] ); }
  bool find( const probabilisticEffect_t &eff ) const;
  bool insert( const probabilisticEffect_t *effect );
  bool affect( state_t &resulting_state,
               state_t &original_state,
               bool nprec = false ) const;
  void clear( void ) { data_ptr_ = data_; }
  void print( std::ostream &os ) const;
  probabilisticEffectList_t& operator=( const probabilisticEffectList_t &effect );
};


/*******************************************************************************
 *
 * effect (abstract class)
 *
 ******************************************************************************/

class effect_t
{
public:
  effect_t() { }
  virtual ~effect_t() { }
  virtual bool affect( state_t &resulting_state,
                       state_t &original_state,
                       bool nprec = false ) const = 0;
  virtual void collect_prec_atoms( atomList_t &atoms ) const = 0;
  virtual void collect_add_atoms( atomList_t &atoms ) const = 0;
  virtual void collect_del_atoms( atomList_t &atoms ) const = 0;
  virtual void print( std::ostream &os ) const = 0;
};


/*******************************************************************************
 *
 * strips effect
 *
 ******************************************************************************/

class stripsEffect_t : public effect_t
{
  atomList_t add_list_;
  atomList_t del_list_;

public:
  stripsEffect_t() { }
  virtual ~stripsEffect_t() { }

  atomList_t& add_list( void ) { return( add_list_ ); }
  const atomList_t& add_list( void ) const { return( add_list_ ); }
  atomList_t& del_list( void ) { return( del_list_ ); }
  const atomList_t& del_list( void ) const { return( del_list_ ); }
  bool empty( void ) const
    {
      return( (add_list().size() == 0) && (del_list().size() == 0) );
    }
  void insert_add( ushort_t atom ) { add_list_.insert( atom ); }
  void insert_del( ushort_t atom ) { del_list_.insert( atom ); }

  virtual bool affect( state_t &resulting_state,
                       state_t &original_state,
                       bool nprec = false ) const;
  virtual void collect_prec_atoms( atomList_t &atoms ) const;
  virtual void collect_add_atoms( atomList_t &atoms ) const;
  virtual void collect_del_atoms( atomList_t &atoms ) const;
  virtual void print( std::ostream &os ) const;
  bool operator==( const stripsEffect_t &effect ) const;
  stripsEffect_t& operator=( const stripsEffect_t &effect );
};


/*******************************************************************************
 *
 * conditional effect
 *
 ******************************************************************************/

class conditionalEffect_t : public effect_t
{
  atomListList_t prec_list_;
  stripsEffect_t s_effect_;

public:
  conditionalEffect_t()
    {
      notify( this, "conditionalEffect_t::conditionalEffect_t()" );
    }
  virtual ~conditionalEffect_t() { }

  atomListList_t& precondition( void ) { return( prec_list_ ); }
  const atomListList_t& precondition( void ) const { return( prec_list_ ); }
  stripsEffect_t& s_effect( void ) { return( s_effect_ ); }
  const stripsEffect_t& s_effect( void ) const { return( s_effect_ ); }
  bool empty( void ) const { return( s_effect().empty() ); }

  virtual bool affect( state_t &resulting_state,
                       state_t &original_state,
                       bool nprec = false ) const;
  virtual void collect_prec_atoms( atomList_t &atoms ) const;
  virtual void collect_add_atoms( atomList_t &atoms ) const;
  virtual void collect_del_atoms( atomList_t &atoms ) const;
  virtual void print( std::ostream &os ) const;
  bool operator==( const conditionalEffect_t &effect ) const;
  conditionalEffect_t& operator=( const conditionalEffect_t &effect );
};


/*******************************************************************************
 *
 * deterministic effect = a strips effect and list of conditional effects
 *
 ******************************************************************************/

class deterministicEffect_t : public effect_t
{
  stripsEffect_t s_effect_;
  conditionalEffectList_t c_effect_;

public:
  deterministicEffect_t() { }
  ~deterministicEffect_t();

  stripsEffect_t& s_effect( void ) { return( s_effect_ ); }
  const stripsEffect_t& s_effect( void ) const { return( s_effect_ ); }
  conditionalEffectList_t& c_effect( void ) { return( c_effect_ ); }
  const conditionalEffectList_t& c_effect( void ) const { return( c_effect_ ); }
  bool empty( void ) const;
  void insert_add( ushort_t atom ) { s_effect_.insert_add( atom ); }
  void insert_del( ushort_t atom ) { s_effect_.insert_del( atom ); }
  void insert_effect( const stripsEffect_t &seff );
  void delete_del_list( void ) { s_effect().del_list().clear(); }

  virtual bool affect( state_t &resulting_state,
                       state_t &original_state,
                       bool nprec = false ) const;
  virtual void collect_prec_atoms( atomList_t &atoms ) const;
  virtual void collect_add_atoms( atomList_t &atoms ) const;
  virtual void collect_del_atoms( atomList_t &atoms ) const;
  virtual void print( std::ostream &os ) const;
  bool operator==( const deterministicEffect_t &effect ) const;
  deterministicEffect_t& operator=( const deterministicEffect_t &effect );
};


/*******************************************************************************
 *
 * probabilistic effect
 *
 ******************************************************************************/

class probabilisticEffect_t : public deterministicEffect_t
{
  Rational probability_;

public:
  probabilisticEffect_t( Rational p ) : probability_(p)
    {
      notify( this, "probabilisticEffect_t::probabilisticEffect_t(Rational)" );
    }
  ~probabilisticEffect_t() { }

  Rational probability( void ) const { return( probability_ ); }
  void increase_probability( Rational p ) { probability_ = probability_ + p; }

  virtual bool affect( state_t &resulting_state,
                       state_t &original_state,
                       bool nprec = false ) const;
  virtual void print( std::ostream &os ) const;
  bool operator==( const probabilisticEffect_t &effect ) const;
  probabilisticEffect_t& operator=( const probabilisticEffect_t &effect );
};


/*******************************************************************************
 *
 * action (abstract class)
 *
 ******************************************************************************/

class action_t
{
  mutable size_t ref_count_;
  const char *name_, *nameXML_;
  atomListList_t precondition_;

protected:
  action_t( const std::string &name, const std::string &nameXML );

public:
  action_t() : ref_count_(0) { }
  virtual ~action_t();

  static void register_use( const action_t *a )
    {
#ifdef MEM_DEBUG
      if(a) std::cerr << "<act>: inc-ref-count " << a << " = " << a->ref_count_+1 << std::endl;
#endif
      if( a != NULL ) ++a->ref_count_;
    }
  static void unregister_use( const action_t *a )
    {
#ifdef MEM_DEBUG
      if(a) std::cerr << "<act>: dec-ref-count " << a << " = " << a->ref_count_-1 << std::endl;
#endif
      if( a && (--a->ref_count_ == 0) ) delete a;
    }

  atomListList_t& precondition( void ) { return( precondition_ ); }
  const atomListList_t& precondition( void ) const { return( precondition_ ); }
  const char* name( void ) const { return( name_ ); }
  const char* nameXML( void ) const { return( nameXML_ ); }
  void print( std::ostream &os ) const { os << name(); }
  void printXML( std::ostream &os ) const { os << nameXML(); }
  bool enabled( const state_t& state, bool nprec = false ) const
    {
      return( precondition().holds( state, nprec ) );
    }
  double cost( const state_t& state ) const { return( 1 ); }

  void insert_precondition( const atomList_t &alist );
  void insert_precondition( const atomListList_t &alist );

  virtual bool empty( void ) const = 0;
  virtual bool affect( state_t& state, bool nprec = false ) const = 0;
  virtual const stateProbList_t& expand( const state_t &state, bool nprec = false ) const = 0;
  virtual void expand( const state_t &state, std::pair<state_t*,Rational> *list,
		       bool nprec = false ) const = 0;
  virtual void print_full( std::ostream &os ) const = 0;
  virtual action_t* clone( void ) const = 0;
  virtual void collect_prec_atoms( atomList_t &atoms ) const = 0;
  virtual void collect_add_atoms( atomList_t &atoms ) const = 0;
  virtual void collect_del_atoms( atomList_t &atoms ) const = 0;
};

class actionList_t : public std::vector<const action_t*> { };


/*******************************************************************************
 *
 * deterministic action
 *
 ******************************************************************************/

class deterministicAction_t : public action_t
{
  deterministicEffect_t effect_;

public:
  deterministicAction_t( const std::string &name, const std::string &nameXML );
  virtual ~deterministicAction_t();

  deterministicEffect_t& effect( void ) { return( effect_ ); }
  const deterministicEffect_t& effect( void ) const { return( effect_ ); }
  void set_effect( const deterministicEffect_t &eff ) { effect() = eff; }
  void insert_add( ushort_t atom ) { effect().insert_add( atom ); }
  void insert_del( ushort_t atom ) { effect().insert_del( atom ); }
  void insert_effect( const stripsEffect_t &seff ) { effect().insert_effect( seff ); }
  void delete_del_list( void ) { effect().delete_del_list(); }

  virtual bool empty( void ) const { return( effect().empty() ); }
  virtual bool affect( state_t& state, bool nprec = false ) const;
  virtual const stateProbList_t& expand( const state_t &state, bool nprec = false ) const;
  virtual void expand( const state_t &state, std::pair<state_t*,Rational> *list,
		       bool nprec = false ) const;
  virtual void print_full( std::ostream &os ) const;
  virtual action_t* clone( void ) const;
  virtual void collect_prec_atoms( atomList_t &atoms ) const;
  virtual void collect_add_atoms( atomList_t &atoms ) const;
  virtual void collect_del_atoms( atomList_t &atoms ) const;

  friend const action_t& Action::translate( const problem_t &problem ) const;
};


/*******************************************************************************
 *
 * probabilistic action
 *
 ******************************************************************************/

class probabilisticAction_t : public action_t
{
  probabilisticEffectList_t effect_list_;

public:
  probabilisticAction_t( const std::string &name, const std::string &nameXML );
  virtual ~probabilisticAction_t();

  probabilisticEffectList_t& effect( void ) { return( effect_list_ ); }
  const probabilisticEffectList_t& effect( void ) const { return( effect_list_ ); }
  bool insert_effect( const probabilisticEffect_t *eff )
    {
      return( effect().insert( eff ) );
    }
  size_t size( void ) const { return( effect().size() ); }
  const probabilisticEffect_t& effect( size_t i ) const { return( effect().effect( i ) ); }
  Rational probability( size_t i ) const { return( effect().effect( i ).probability() ); }

  virtual bool empty( void ) const;
  virtual bool affect( state_t& state, bool nprec = false ) const;
  virtual const stateProbList_t& expand( const state_t &state, bool nprec = false ) const;
  virtual void expand( const state_t &state, std::pair<state_t*,Rational> *list,
		       bool nprec = false ) const;
  virtual void print_full( std::ostream &os ) const;
  virtual action_t* clone( void ) const;
  virtual void collect_prec_atoms( atomList_t &atoms ) const;
  virtual void collect_add_atoms( atomList_t &atoms ) const;
  virtual void collect_del_atoms( atomList_t &atoms ) const;
};


/*******************************************************************************
 *
 * misc. inline functions
 *
 ******************************************************************************/

inline bool
conditionalEffectList_t::find( const conditionalEffect_t &eff ) const
{
  for( size_t i = 0; i < size(); ++i )
    if( effect( i ) == eff ) return( true );
  return( false );
}

inline void
conditionalEffectList_t::insert( const conditionalEffect_t *effect )
{
  if( !find( *effect ) )
    {
      if( !data_ || (data_ptr_ == &data_[size_]) )
	{
	  size_ = (!data_ ? 1 : size_ << 1);
	  const conditionalEffect_t **ndata_ =
	    (const conditionalEffect_t**)
	    realloc( data_, size_ * sizeof(conditionalEffect_t*) );
	  data_ptr_ = (!data_ ? ndata_ : &ndata_[data_ptr_ - data_]);
	  data_ = ndata_;
	}
      *data_ptr_++ = effect;
    }
}

inline bool
probabilisticEffectList_t::find( const probabilisticEffect_t &eff ) const
{
  for( size_t i = 0; i < size(); ++i )
    if( effect( i ) == eff ) return( true );
  return( false );
}

inline bool
probabilisticEffectList_t::insert( const probabilisticEffect_t *eff )
{
  size_t i;
  for( i = 0; i < size(); ++i )
    if( effect( i ) == *eff ) break;

  if( i == size() )
    {
      if( !data_ || (data_ptr_ == &data_[size_]) )
	{
	  size_ = (!data_ ? 1 : size_ << 1);
	  const probabilisticEffect_t **ndata_ =
	    (const probabilisticEffect_t**)
	    realloc( data_, size_ * sizeof(const probabilisticEffect_t*) );
	  data_ptr_ = (!data_ ? ndata_ : &ndata_[data_ptr_ - data_]);
	  data_ = ndata_;
	}
      *data_ptr_++ = eff;
      return( true );
    }
  else
    {
      // this is a violation of the const abstraction
      ((probabilisticEffect_t*)data_[i])->increase_probability( eff->probability() );
      return( false );
    }
}

#endif // ACTIONS_H
