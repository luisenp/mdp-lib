#ifndef PROBLEMS_T_H
#define PROBLEMS_T_H

#include "global.h"
#include "actions.h"
#include "domains.h"
#include "effects.h"
#include "formulas.h"
#include "graph.h"
#include "expressions.h"
#include "terms.h"
#include "types.h"

#include <iostream>
#include <map>
#include <string>
#include <vector>

class Application;
class Atom;
class Domain;
class Expression;
class StateFormula;

class state_t;
class stateProbList_t;


/*******************************************************************************
 *
 * system_t
 *
 ******************************************************************************/

class system_t
{
 public:
  const std::vector<const std::string*> *focus_;
  const std::vector<const std::string*> *base_;
  const std::vector<const std::string*> *frame_;

  system_t( const std::vector<const std::string*> *focus,
	    const std::vector<const std::string*> *base,
	    const std::vector<const std::string*> *frame )
    : focus_(focus), base_(base), frame_(frame) { }
  ~system_t() { delete focus_; delete base_; delete frame_; }

  const std::vector<const std::string*>& focus( void ) const { return( *focus_ ); }
  const std::vector<const std::string*>& base( void ) const { return( *base_ ); }
  const std::vector<const std::string*>& frame( void ) const { return( *frame_ ); }
};


/*******************************************************************************
 *
 * problem_t
 *
 ******************************************************************************/

class problem_t
{
public:
  typedef enum { MINIMIZE_EXPECTED_COST, MAXIMIZE_GOAL_PROBABILITY } metric_t;

private:
  mutable size_t ref_count_;

  class ProblemMap : public std::map<std::string,problem_t*> { };

  static ProblemMap problems;
  static ProblemMap::const_iterator begin( void ) { return( problems.begin() ); }
  static ProblemMap::const_iterator end( void ) { return( problems.end() ); }

  static ushort_t atom_index_;
  static bool no_more_atoms_;
  static std::map<const Atom*,ushort_t> atom_hash_;
  static std::map<ushort_t,const Atom*> atom_inv_hash_;

  static ushort_t fluent_index_;
  static std::map<const Application*,ushort_t> fluent_hash_;
  static std::map<ushort_t,const Application*> fluent_inv_hash_;

  std::string name_;
  const Domain* domain_;
  TermTable terms_;
  AtomSet init_atoms_;
  ValueMap init_fluents_;
  EffectList init_effects_;
  const StateFormula* goal_; // NULL if not a goal-directed planning problem.
  metric_t metric_;
  ActionList actions_;

  bool nprec_;
  bool goal_atom_;
  atomListList_t goalT_;
  actionList_t actionsT_;
  digraph_t *application_graph_;
  std::vector<const atomList_t*> restriction_;
  std::map<const StateFormula*,const Atom*> instantiated_hash_;

  std::map<const std::string,std::vector<const Atom*>*> orbits_;
  std::map<const std::string,const system_t*> systems_;

  static problem_t* find_problem( const std::string &name );
  static problem_t* allocate( const std::string &name, const problem_t &problem );

  static void compute_weak_relaxation( problem_t &problem, bool verb );
  static void compute_medium_relaxation( problem_t &problem, bool verb );
  static void compute_strong_relaxation( problem_t &problem, bool verb );
  static void subsets( size_t i, const conditionalEffectList_t &ceff_set,
		       std::list<const conditionalEffect_t*> &tmp_set,
		       std::list<std::list<const conditionalEffect_t*>*> &result );

  explicit problem_t( const std::string &name, const problem_t &problem );

  friend std::ostream& operator<<( std::ostream& os, const problem_t& p );

public:
  problem_t( const std::string &name, const Domain &domain );
  ~problem_t();

  static void register_use( const problem_t *p )
    {
#ifdef MEM_DEBUG
      if(p) std::cerr << "<pro>: inc-ref-count " << p << " = " << p->ref_count_+1 << std::endl;
#endif
      if( p ) ++p->ref_count_;
    }
  static void unregister_use( const problem_t *p )
    {
#ifdef MEM_DEBUG
      if(p) std::cerr << "<pro>: dec-ref-count " << p << " = " << p->ref_count_-1 << std::endl;
#endif
      if( p && (--p->ref_count_ == 0) ) delete p;
    }

  static problem_t* allocate( const std::string &name, const Domain &domain );
  static const problem_t* find( const std::string &name );
  static void clear( void );
  static ushort_t number_atoms( void ) { return( atom_index_ ); }
  static ushort_t number_fluents( void ) { return( fluent_index_ ); }
  static ushort_t atom_hash_get( const Atom &atom, bool negated = false );
  static ushort_t atom_get_new( void )
    {
      ushort_t atm = atom_index_;
      atom_index_ += 2;
      return( atm );
    }
  static void no_more_atoms( void ) { no_more_atoms_ = true; }
  static ushort_t fluent_hash_get( const Application &app );
  static const Atom* atom_inv_hash_get( ushort_t atom );
  static const Application* fluent_inv_hash_get( ushort_t fluent );

  const std::string& name( void ) const { return( name_ ); }
  const Domain& domain( void ) const { return( *domain_ ); }
  TermTable& terms( void ) { return( terms_ ); }
  const TermTable& terms( void ) const { return( terms_ ); }

  void add_init_atom( const Atom& atom );
  void add_init_fluent( const Application &application, const Rational &value );
  void add_init_effect( const Effect &effect );
  void set_goal( const StateFormula &goal );
  void set_metric( metric_t metric ) { metric_ = metric; }

  void instantiate_actions( void );
  void flatten( void );
  const problem_t& weak_relaxation( void ) const;
  const problem_t& medium_relaxation( void ) const;
  const problem_t& strong_relaxation( void ) const;

  // Fills the provided object list with objects (including constants
  // declared in the domain) that are compatible with the given type.
  void compatible_objects( ObjectList &objects, Type type ) const;

  const AtomSet& init_atoms( void ) const { return( init_atoms_ ); }
  const ValueMap& init_fluents( void ) const { return( init_fluents_ ); }
  const EffectList& init_effects( void ) const { return( init_effects_ ); }
  const StateFormula& goal( void ) const { return( *goal_ ); }
  const metric_t metric( void ) const { return( metric_ ); }
  const ActionList& actions( void ) const { return( actions_ ); }

  bool nprec( void ) const { return( nprec_ ); }
  bool goal_atom( void ) const { return( goal_atom_ ); }
  const atomListList_t& goalT( void ) const { return( goalT_ ); }
  atomListList_t& goalT( void ) { return( goalT_ ); }
  const actionList_t& actionsT( void ) const { return( actionsT_ ); }
  actionList_t& actionsT( void ) { return( actionsT_ ); }
  const atomList_t& restriction( unsigned action ) const
    {
      return( *restriction_[action] );
    }
  void complete_state( state_t &state ) const;

  void enabled_actions( ActionList& actions, const state_t& state ) const;
  void print( std::ostream &os, const StateFormula &formula ) const;
  void print( std::ostream &os, const Application &app ) const;
  void print( std::ostream &os, const Action &action ) const;
  void print_full( std::ostream &os, const Action &action ) const;

  const stateProbList_t& expand( const action_t &action, const state_t &state ) const;
  void expand( const action_t &action, const state_t &state,
	       std::pair<state_t*,Rational> *list ) const;
  void initial_states( std::pair<state_t*,Rational> *list ) const;

  void analyze_symmetries( void );

  void add_orbit( std::string *name, std::vector<const Atom*> *atoms );
  void add_system( std::string *name,
		   std::vector<const std::string*> *focus,
		   std::vector<const std::string*> *base,
		   std::vector<const std::string*> *frame );
  const std::map<const std::string,std::vector<const Atom*>*>& orbits( void ) const { return( orbits_ ); }
  const std::map<const std::string,const system_t*>& systems( void ) const { return( systems_ ); }

  static std::map<const Atom*,ushort_t>& atom_hash() { return atom_hash_;}
};

std::ostream& operator<<( std::ostream& os, const problem_t& p );

#endif // PROBLEMS_T_H
