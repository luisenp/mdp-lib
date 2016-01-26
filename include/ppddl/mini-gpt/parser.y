%{
#include "global.h"
#include "problems.h"
#include "domains.h"
#include "actions.h"
#include "effects.h"
#include "formulas.h"
#include "expressions.h"
#include "functions.h"
#include "predicates.h"
#include "terms.h"
#include "types.h"
#include "rational.h"
#include "exceptions.h"
#include <iostream>
#include <map>
#include <typeinfo>

// context of free variables.
class context_t
{
  struct VariableMap : public std::map<std::string,Variable> { };
  std::vector<VariableMap> frames_;

public:
  void push_frame() { frames_.push_back(VariableMap()); }
  void pop_frame() { frames_.pop_back(); }
  void insert( const std::string &name, Variable v )
  {
    frames_.back()[name] = v;
  }
  std::pair<Variable,bool> shallow_find( const std::string &name ) const
  {
    VariableMap::const_iterator vi = frames_.back().find( name );
    if( vi != frames_.back().end() )
      return( std::make_pair( (*vi).second, true ) );
    else
      return( std::make_pair( 0, false ) );
  }
  std::pair<Variable,bool> find( const std::string &name ) const
  {
    for( std::vector<VariableMap>::const_reverse_iterator fi =
	   frames_.rbegin(); fi != frames_.rend(); ++fi )
      {
	VariableMap::const_iterator vi = (*fi).find( name );
	if( vi != (*fi).end() )
	  return( std::make_pair( (*vi).second, true ) );
      }
    return( std::make_pair( 0, false ) );
  }
};

extern int yylex();
extern size_t line_number;
extern char *current_file;
static bool success = true;

static Domain* domain;
static std::map<std::string,Domain*> domains;
static problem_t* problem = NULL;
static Requirements* requirements;

static Function goal_prob_function; 
static Function reward_function;
static Predicate predicate;
static bool parsing_predicate;
static bool repeated_predicate;
static Function function;
static bool parsing_function;
static bool repeated_function;

static ActionSchema* action;
static context_t context;

// predicate for atomic state formula being parsed.
static Predicate atom_predicate;
// whether the predicate of the currently parsed atom was undeclared.
static bool undeclared_atom_predicate;
// whether parsing effect fluents.
static bool effect_fluent;
// function for function application being parsed.
static Function appl_function;
// whether the function of the currently parsed application was undeclared.
static bool undeclared_appl_function;
// paramerers for atomic state formula or function application being parsed.
static TermList term_parameters;
// whether parsing an atom.
static bool parsing_atom;
// whether parsing a function application.
static bool parsing_application;
// quantified variables for effect or formula being parsed.
static VariableList quantified;
// most recently parsed term for equality formula.
static Term eq_term;
// most recently parsed expression for equality formula.
static const Expression* eq_expr;
// the first term for equality formula.
static Term first_eq_term;
// the first expression for equality formula.
static const Expression* first_eq_expr;
// kind of name map being parsed.
static enum { TYPE_KIND, CONSTANT_KIND, OBJECT_KIND, VOID_KIND } name_kind;

static void yyerror( const std::string &s ); 
static void yywarning( const std::string &s );
static void make_domain( const std::string *name );
static void make_problem( const std::string *name, const std::string *domain_name );
static void require_typing( void );
static void require_fluents( void );
static void require_disjunction( void );
static void require_conditional_effects( void );

// returns a simple type with the given name.
static Type make_type( const std::string *name );
// returns the union of the given types.
static Type make_type( const TypeSet& types );
static Term make_term( const std::string *name );

static void make_predicate( const std::string *name );
static void make_function( const std::string *name );
static void make_action( const std::string *name );
static void add_action( void );
static void prepare_forall_effect( void );
static const Effect* make_forall_effect( const Effect& effect );

// adds an outcome to the given probabilistic effect.
static void add_effect_outcome( ProbabilisticEffect& peffect, 
				const Rational* p, const Effect& effect );
static const Effect* make_add_effect( const Atom& atom );
static const Effect* make_delete_effect( const Atom& atom );
static const Effect* make_assignment_effect( unsigned oper,
					     const Application& application,
					     const Expression& expr );

// adds types, constants, or objects to the current domain or problem.
static void add_names( const std::vector<const std::string*>* names, Type type );
static void add_variables( const std::vector<const std::string*>* names, Type type );
static void prepare_atom( const std::string *name );
static void prepare_application( const std::string *name );
static void add_term( const std::string *name );
static const Atom* make_atom( void );
static const Application* make_application( void );
static const Expression* make_subtraction( const Expression& term, const Expression* opt_term );

// creates an atom or fluent for the given name to be used in an equality formula.
static void make_eq_name( const std::string *name );
static const StateFormula* make_equality( void );
static const StateFormula* make_negation( const StateFormula& negand );
static const StateFormula* make_implication( const StateFormula& f1, const StateFormula& f2 );
static void prepare_exists( void );
static void prepare_forall( void );
static const StateFormula* make_exists( const StateFormula& body );
static const StateFormula* make_forall( const StateFormula& body );
%}

%token DEFINE DOMAIN_TOKEN PROBLEM
%token REQUIREMENTS TYPES CONSTANTS PREDICATES FUNCTIONS
%token STRIPS TYPING NEGATIVE_PRECONDITIONS DISJUNCTIVE_PRECONDITIONS EQUALITY
%token EXISTENTIAL_PRECONDITIONS UNIVERSAL_PRECONDITIONS
%token QUANTIFIED_PRECONDITIONS CONDITIONAL_EFFECTS FLUENTS ADL
%token DURATIVE_ACTIONS DURATION_INEQUALITIES CONTINUOUS_EFFECTS
%token PROBABILISTIC_EFFECTS REWARDS MDP
%token ACTION PARAMETERS PRECONDITION EFFECT
%token PDOMAIN OBJECTS INIT GOAL GOAL_REWARD METRIC EXPECTED_COST GOAL_PROBABILITY
%token WHEN NOT AND OR IMPLY EXISTS FORALL PROBABILISTIC
%token ASSIGN SCALE_UP SCALE_DOWN INCREASE DECREASE MINIMIZE MAXIMIZE
%token NUMBER_TOKEN OBJECT_TOKEN EITHER
%token LT LE GE GT NAME VARIABLE NUMBER
%token ILLEGAL_TOKEN
%token PLUS HYPHEN STAR SLASH
%token LEFTPAR RIGHTPAR LEFTBRACE RIGHTBRACE EQUAL
%token ORBIT SYSTEM

%union
{
  unsigned setop;
  const Effect* effect;
  ConjunctiveEffect* ceffect;
  ProbabilisticEffect* peffect;
  const StateFormula* formula;
  const Atom* atom;
  Conjunction* conj;
  Disjunction* disj;
  const Expression* expr;
  const Application* appl;
  Comparison::CmpPredicate comp;
  Type type;
  TypeSet* types;
  std::string* str;
  std::vector<const std::string*>* strs;
  std::vector<const Atom*>* atoms;
  const Rational* num;
}

%type <setop> assign_op
%type <effect> eff_formula p_effect simple_init one_init
%type <ceffect> eff_formulas one_inits
%type <peffect> prob_effs prob_inits
%type <formula> formula
%type <atom> atomic_name_formula atomic_term_formula
%type <conj> conjuncts
%type <disj> disjuncts
%type <expr> value f_exp opt_f_exp
%type <appl> ground_f_head f_head
%type <comp> binary_comp
%type <strs> name_seq variable_seq
%type <type> type_spec type
%type <types> types
%type <str> type_name predicate function name variable
%type <str> DEFINE DOMAIN_TOKEN PROBLEM GOAL_PROBABILITY
%type <str> WHEN NOT AND OR IMPLY EXISTS FORALL PROBABILISTIC
%type <str> ASSIGN SCALE_UP SCALE_DOWN INCREASE DECREASE MINIMIZE MAXIMIZE
%type <str> NUMBER_TOKEN OBJECT_TOKEN EITHER
%type <str> NAME VARIABLE
%type <num> probability NUMBER
%type <atoms> atoms
%type <strs> orbits orbit_set

%%

file                :   {
                          success = true;
			  line_number = 1;
			}
                      domains_and_problems
		        {
			  if( !success )
			    YYERROR;
			}
                    ;

domains_and_problems
                    : domains_and_problems domain_def
                    | domains_and_problems problem_def
                    | /* empty */
                    ;

domain_def          : LEFTPAR define LEFTPAR domain name RIGHTPAR
                        {
			  make_domain( $5 );
			}
                      domain_body RIGHTPAR
                    ;

domain_body         : require_def
                    | require_def domain_body2
                    | domain_body2
                    | /* empty */
                    ;

domain_body2        : types_def
                    | types_def domain_body3
                    | domain_body3
                    ;

domain_body3        : constants_def
                    | predicates_def
                    | functions_def
                    | constants_def domain_body4
                    | predicates_def domain_body5
                    | functions_def domain_body6
                    | structure_defs
                    ;

domain_body4        : predicates_def
                    | functions_def
                    | predicates_def domain_body7
                    | functions_def domain_body8
                    | structure_defs
                    ;

domain_body5        : constants_def
                    | functions_def
                    | constants_def domain_body7
                    | functions_def domain_body9
                    | structure_defs
                    ;

domain_body6        : constants_def
                    | predicates_def
                    | constants_def domain_body8
                    | predicates_def domain_body9
                    | structure_defs
                    ;

domain_body7        : functions_def 
                    | functions_def structure_defs
                    | structure_defs
                    ;

domain_body8        : predicates_def
                    | predicates_def structure_defs
                    | structure_defs
                    ;

domain_body9        : constants_def
                    | constants_def structure_defs
                    | structure_defs
                    ;

structure_defs      : structure_def
                    | structure_defs structure_def
                    ;

structure_def       : action_def
                    ;

require_def         : LEFTPAR REQUIREMENTS require_keys RIGHTPAR
                    ;

require_keys        : require_key
                    | require_keys require_key
                    ;

require_key         : STRIPS
                        {
			  requirements->strips = true;
			}
                    | TYPING
                        {
			  requirements->typing = true;
			}
                    | NEGATIVE_PRECONDITIONS
                        {
			  requirements->negative_preconditions = true;
			}
                    | DISJUNCTIVE_PRECONDITIONS
                        {
			  requirements->disjunctive_preconditions = true;
			}
                    | EQUALITY
                        {
			  requirements->equality = true;
			}
                    | EXISTENTIAL_PRECONDITIONS
                        {
			  requirements->existential_preconditions = true;
			}
                    | UNIVERSAL_PRECONDITIONS
                        {
			  requirements->universal_preconditions = true;
			}
                    | QUANTIFIED_PRECONDITIONS
                        {
			  requirements->quantified_preconditions();
			}
                    | CONDITIONAL_EFFECTS
                        {
			  requirements->conditional_effects = true;
			}
                    | FLUENTS
                        {
			  requirements->fluents = true;
			}
                    | ADL
                        {
			  requirements->adl();
			}
                    | DURATIVE_ACTIONS
                        {
			  throw Exception( "`:durative-actions' not supported" );
			}
                    | DURATION_INEQUALITIES
                        {
			  throw Exception( "`:duration-inequalities' not supported" );
			}
                    | CONTINUOUS_EFFECTS
                        {
			  throw Exception( "`:continuous-effects' not supported" );
			}
                    | PROBABILISTIC_EFFECTS
                        {
			  requirements->probabilistic_effects = true;
			  goal_prob_function =
			    domain->functions().add_function( "goal-probability" );
			}
                    | REWARDS
                        {
			  requirements->rewards = true;
			  reward_function = domain->functions().add_function( "reward" );
			}
                    | MDP
                        {
			  requirements->probabilistic_effects = true;
			  requirements->rewards = true;
			  goal_prob_function =
			    domain->functions().add_function( "goal-probability" );
			  reward_function = domain->functions().add_function( "reward" );
			}
                    ;

types_def           : LEFTPAR TYPES
                        {
			  require_typing();
			  name_kind = TYPE_KIND;
			}
                      typed_names RIGHTPAR
                        {
			  name_kind = VOID_KIND;
			}
                    ;

constants_def       : LEFTPAR CONSTANTS
                        {
			  name_kind = CONSTANT_KIND;
			}
                      typed_names RIGHTPAR
                        {
			  name_kind = VOID_KIND;
			}
                    ;

predicates_def      : LEFTPAR PREDICATES predicate_decls RIGHTPAR
                    ;

functions_def       : LEFTPAR FUNCTIONS
                        {
			  require_fluents();
			}
                      function_decls RIGHTPAR
                    ;

predicate_decls     : predicate_decls predicate_decl
                    | /* empty */
                    ;

predicate_decl      : LEFTPAR predicate
                        {
			  make_predicate( $2 );
			}
                      variables RIGHTPAR
                        {
			  parsing_predicate = false;
			}
                    ;

function_decls      : function_decl_seq
                    | function_decl_seq function_type_spec function_decls
                    | /* empty */
                    ;

function_decl_seq   : function_decl
                    | function_decl_seq function_decl
                    ;

function_type_spec  : HYPHEN
                        {
			  require_typing();
			}
                      function_type
                    ;

function_decl       : LEFTPAR function
                        {
			  make_function( $2 );
			}
                      variables RIGHTPAR
                        {
			  parsing_function = false;
			}
                    ;

action_def          : LEFTPAR ACTION name
                        {
			  make_action( $3 );
			}
                      parameters action_body RIGHTPAR
                        {
			  add_action();
			}
                    ;

parameters          : PARAMETERS LEFTPAR variables RIGHTPAR
                    | /* empty */
                    ;

action_body         : precondition action_body2
                    | action_body2
                    ;

action_body2        : effect
                    | /* empty */
                    ;

precondition        : PRECONDITION formula
                        {
			  action->set_precondition( *$2 );
			  StateFormula::unregister_use( $2 );
			}
                    ;

effect              : EFFECT eff_formula
                        {
			  action->set_effect( *$2 );
			  Effect::unregister_use( $2 );
			}
                    ;

eff_formula         : p_effect
                    | LEFTPAR and eff_formulas RIGHTPAR
                        {
			  $$ = $3;
			}
                    | LEFTPAR forall
                        {
			  prepare_forall_effect();
			}
                      LEFTPAR variables RIGHTPAR eff_formula RIGHTPAR
                        {
			  $$ = make_forall_effect( *$7 );
			}
                    | LEFTPAR when
                        {
			  require_conditional_effects();
			}
                      formula eff_formula RIGHTPAR
                        {
			  $$ = &ConditionalEffect::make( *$4, *$5 );
			}
                    | LEFTPAR probabilistic prob_effs RIGHTPAR
                        {
			  $$ = $3;
			}
                    ;

eff_formulas        : eff_formulas eff_formula
                        {
			  $$ = $1;
			  $$->add_conjunct( *$2 );
			}
                    | /* empty */
                        {
			  $$ = new ConjunctiveEffect;
			}
                    ;

prob_effs           : probability eff_formula
                        {
			  $$ = new ProbabilisticEffect;
			  add_effect_outcome( *$$, $1, *$2 );
			}
                    | prob_effs probability eff_formula
                        {
			  $$ = $1;
			  add_effect_outcome( *$$, $2, *$3 );
			}
                    ;

probability         : NUMBER
                    ;

p_effect            : atomic_term_formula
                        {
			  $$ = make_add_effect( *$1 );
			}
                    | LEFTPAR not atomic_term_formula RIGHTPAR
                        {
			  $$ = make_delete_effect( *$3 );
			}
                    | LEFTPAR assign_op
                        {
			  effect_fluent = true;
			}
                      f_head f_exp RIGHTPAR
                        {
			  $$ = make_assignment_effect( $2, *$4, *$5 );
			}
                    ;

assign_op           : assign
                        {
			  $$ = Assignment::ASSIGN_OP;
			}
                    | scale_up
                        {
			  $$ = Assignment::SCALE_UP_OP;
			}
                    | scale_down
                        {
			  $$ = Assignment::SCALE_DOWN_OP;
			}
                    | increase
                        {
			  $$ = Assignment::INCREASE_OP;
			}
                    | decrease
                        {
			  $$ = Assignment::DECREASE_OP;
			}
                    ;

problem_def         : LEFTPAR define LEFTPAR problem name RIGHTPAR
                      LEFTPAR PDOMAIN name RIGHTPAR
                        {
			  make_problem( $5, $9 );
			}
                      problem_body RIGHTPAR
                        {
			  delete requirements;
			}
                    ;

problem_body        : require_def problem_body2
                    | problem_body2
                    ;

problem_body2       : object_decl problem_body3
                    | problem_body3
                    ;

problem_body3       : init problem_body4
                    | problem_body4
                    ;

problem_body4       : goal problem_body5
                    | problem_body5
                    ;

problem_body5       : goal_reward problem_body6
                    | problem_body6
                    ;

problem_body6       : metric_spec problem_body7
                    | problem_body7
                    ;

problem_body7       : orbit_decls problem_body8
                    | problem_body8
                    ;

problem_body8       : system_decls
                    | /* empty */
                    ;

object_decl         : LEFTPAR OBJECTS
                        {
			  name_kind = OBJECT_KIND;
			}
                      typed_names RIGHTPAR
                        {
			  name_kind = VOID_KIND;
			}
                    ;

init                : LEFTPAR INIT init_elements RIGHTPAR
                    ;

init_elements       : init_elements init_element
                    | /* empty */
                    ;

init_element        : atomic_name_formula
                        {
			  problem->add_init_atom( *$1 );
			  StateFormula::unregister_use( $1 );
			}
                    | LEFTPAR EQUAL ground_f_head NUMBER RIGHTPAR
                        {
			  problem->add_init_fluent( *$3, *$4 );
			  delete $4;
			}
                    | LEFTPAR probabilistic prob_inits RIGHTPAR
                        {
			  problem->add_init_effect( *$3 );
			  Effect::unregister_use( $3 );
			}
                    ;

prob_inits          : probability simple_init
                        {
			  $$ = new ProbabilisticEffect;
			  add_effect_outcome( *$$, $1, *$2 );
			}
                    | prob_inits probability simple_init
                        {
			  $$ = $1;
			  add_effect_outcome( *$$, $2, *$3 );
			}
                    ;

simple_init         : one_init
                    | LEFTPAR and one_inits RIGHTPAR
                        {
			  $$ = $3;
			}
                    ;

one_inits           : one_inits one_init
                        {
			  $$ = $1;
			  $$->add_conjunct( *$2 );
			}
                    | /* empty */
                        {
			  $$ = new ConjunctiveEffect;
			}
                    ;

one_init            : atomic_name_formula
                        {
			  $$ = make_add_effect( *$1 );
			}
                    | LEFTPAR EQUAL ground_f_head value RIGHTPAR
                        {
			  $$ = make_assignment_effect( Assignment::ASSIGN_OP, *$3, *$4 );
			}
                    ;

value               : NUMBER
                        {
			  $$ = new Value( *$1 );
			  delete $1;
			}
                    ;

goal                : LEFTPAR GOAL formula RIGHTPAR
                        {
			  problem->set_goal( *$3 );
			  StateFormula::unregister_use( $3 );
			}
                    ;

goal_reward         : LEFTPAR GOAL_REWARD value RIGHTPAR
                    ;

metric_spec         : LEFTPAR METRIC maximize GOAL_PROBABILITY RIGHTPAR
                        {
			  /* don't care about rewards (blai) */
			  /* problem->set_metric( problem_t::MAXIMIZE_GOAL_PROBABILITY ); */
			}
                    | LEFTPAR METRIC minimize EXPECTED_COST RIGHTPAR
                        {
			  /* don't care about rewards (blai) */
			  /* problem->set_metric( problem_t::MINIMIZE_EXPECTED_COST ); */
			}
                    | LEFTPAR METRIC maximize f_head RIGHTPAR
                    ;

orbit_decls         : orbit_decls orbit_decl
                    | orbit_decl
                    ;

orbit_decl          : LEFTPAR ORBIT NAME atoms RIGHTPAR
                        {
			  problem->add_orbit( $3, $4 );
			  delete $3;
			}
                    ;

atoms               : atoms atomic_name_formula
                        {
			  $1->push_back( $2 );
			  $$ = $1;
			}
                    | atomic_name_formula
                        {
			  $$ = new std::vector<const Atom*>;
			  $$->push_back( $1 );
			}
                    ;

system_decls        : system_decls system_decl
                    | system_decl
                    ;

system_decl         : LEFTPAR SYSTEM NAME orbit_set orbit_set orbit_set RIGHTPAR
                        {
			  problem->add_system( $3, $4, $5, $6 );
			}
                    ;

orbit_set           : LEFTBRACE orbits RIGHTBRACE
                        {
			  $$ = $2;
			}
                    ;

orbits              : orbits NAME
                        {
			  $1->push_back( $2 );
			  $$ = $1;
			}
                    | /* empty */
                        {
			  $$ = new std::vector<const std::string*>;
			}
                    ;

formula             : atomic_term_formula
                        {
			  $$ = $1;
			}
                    | LEFTPAR EQUAL term_or_f_exp
                        {
			  first_eq_term = eq_term;
			  first_eq_expr = eq_expr;
			}
                      term_or_f_exp RIGHTPAR
                        {
			  $$ = make_equality();
			}
                    | LEFTPAR binary_comp
                        {
			  require_fluents();
			}
                      f_exp f_exp RIGHTPAR
                        {
			  $$ = new Comparison( $2, *$4, *$5 );
			  Expression::unregister_use( $4 );
			  Expression::unregister_use( $5 );
			}
                    | LEFTPAR not formula RIGHTPAR
                        {
			  $$ = make_negation( *$3 );
			}
                    | LEFTPAR and conjuncts RIGHTPAR
                        {
			  $$ = $3;
			}
                    | LEFTPAR or
                        {
			  require_disjunction();
			}
                      disjuncts RIGHTPAR
                        {
			  $$ = $4;
			}
                    | LEFTPAR imply formula formula RIGHTPAR
                        {
			  $$ = make_implication( *$3, *$4 );
			}
                    | LEFTPAR exists
                        {
			  prepare_exists();
			}
                      LEFTPAR variables RIGHTPAR formula RIGHTPAR
                        {
			  $$ = make_exists( *$7 );
			}
                    | LEFTPAR forall
                        {
			  prepare_forall();
			}
                      LEFTPAR variables RIGHTPAR formula RIGHTPAR
                        {
			  $$ = make_forall( *$7 );
			}
                    ;

conjuncts           : conjuncts formula
                        {
			  $$->add_conjunct( *$2 );
			}
                    | /* empty */
                        {
			  $$ = new Conjunction;
			}
                    ;

disjuncts           : disjuncts formula
                        {
			  $$->add_disjunct( *$2 );
			}
                    | /* empty */
                        {
			  $$ = new Disjunction;
			}
                    ;

atomic_term_formula : LEFTPAR predicate
                        {
			  prepare_atom( $2 );
			}
                      terms RIGHTPAR
                        {
			  $$ = make_atom();
			}
                    | predicate
                        {
			  prepare_atom( $1 );
			  $$ = make_atom();
			}
                    ;

atomic_name_formula : LEFTPAR predicate
                        {
			  prepare_atom( $2 );
			}
                      names RIGHTPAR
                        {
			  $$ = make_atom();
			}
                    | predicate
                        {
			  prepare_atom( $1 );
			  $$ = make_atom();
			}
                    ;

binary_comp         : LT { $$ = Comparison::LT_CMP; }
                    | LE { $$ = Comparison::LE_CMP; }
                    | GE { $$ = Comparison::GE_CMP; }
                    | GT { $$ = Comparison::GT_CMP; }
                    ;

f_exp               : NUMBER
                        {
			  $$ = new Value( *$1 );
			  delete $1;
			}
                    | LEFTPAR PLUS f_exp f_exp RIGHTPAR
                        {
			  $$ = new Addition( *$3, *$4 );
			  Expression::unregister_use( $3 );
			  Expression::unregister_use( $4 );
			}
                    | LEFTPAR HYPHEN f_exp opt_f_exp RIGHTPAR
                        {
			  $$ = make_subtraction( *$3, $4 );
			  Expression::unregister_use( $3 );
			  Expression::unregister_use( $4 );
			}
                    | LEFTPAR STAR f_exp f_exp RIGHTPAR
                        {
			  $$ = new Multiplication( *$3, *$4 );
			  Expression::unregister_use( $3 );
			  Expression::unregister_use( $4 );
			}
                    | LEFTPAR SLASH f_exp f_exp RIGHTPAR
                        {
			  $$ = new Division( *$3, *$4 );
			  Expression::unregister_use( $3 );
			  Expression::unregister_use( $4 );
			}
                    | f_head
                        {
			  $$ = $1;
			}
                    ;

term_or_f_exp       : NUMBER
                        {
			  require_fluents();
			  eq_expr = new Value( *$1 );
			  delete $1;
			}
                    | LEFTPAR PLUS
                        {
			  require_fluents();
			}
                      f_exp f_exp RIGHTPAR
                        {
			  eq_expr = new Addition( *$4, *$5 );
			  Expression::unregister_use( $4 );
			  Expression::unregister_use( $5 );
			}
                    | LEFTPAR HYPHEN
                        {
			  require_fluents();
			}
                      f_exp opt_f_exp RIGHTPAR
                        {
			  eq_expr = make_subtraction( *$4, $5 );
			}
                    | LEFTPAR STAR
                        {
			  require_fluents();
			}
                      f_exp f_exp RIGHTPAR
                        {
			  eq_expr = new Multiplication( *$4, *$5 );
			  Expression::unregister_use( $4 );
			  Expression::unregister_use( $5 );
			}
                    | LEFTPAR SLASH
                        {
			  require_fluents();
			}
                      f_exp f_exp RIGHTPAR
                        {
			  eq_expr = new Division( *$4, *$5 );
			  Expression::unregister_use( $4 );
			  Expression::unregister_use( $5 );
			}
                    | LEFTPAR function
                        {
			  require_fluents();
			  prepare_application($2);
			}
                      terms RIGHTPAR
                        {
			  eq_expr = make_application();
			}
                    | name
                        {
			  make_eq_name( $1 );
			}
                    | variable
                        {
			  eq_term = make_term( $1 );
			  eq_expr = NULL;
			}
                    ;

opt_f_exp           : f_exp
                    | /* empty */
                        {
			  $$ = NULL;
			}
                    ;

f_head              : LEFTPAR function
                        {
			  prepare_application( $2 );
			}
                      terms RIGHTPAR
                        {
			  $$ = make_application();
			}
                    | function
                        {
			  prepare_application( $1 );
			  $$ = make_application();
			}
                    ;

ground_f_head       : LEFTPAR function
                        {
			  prepare_application( $2 );
			}
                      names RIGHTPAR
                        {
			  $$ = make_application();
			}
                    | function
                        {
			  prepare_application( $1 );
			  $$ = make_application();
			}
                    ;

terms               : terms name
                        {
			  add_term( $2 );
			}
                    | terms variable
                        {
			  add_term( $2 );
			}
                    | /* empty */
		    ;

names               : names name
                        {
			  add_term( $2 );
			}
                    | /* empty */
                    ;

variables           : variable_seq
                        {
			  add_variables( $1, OBJECT_TYPE );
			}
                    | variable_seq type_spec
                        {
			  add_variables( $1, $2 );
			}
                      variables
                    | /* empty */
                    ;

variable_seq        : variable
                        {
			  $$ = new std::vector<const std::string*>( 1, $1 );
			}
                    | variable_seq variable
                        {
			  $$ = $1;
			  $$->push_back( $2 );
			}
                    ;

typed_names         : name_seq
                        {
			  add_names( $1, OBJECT_TYPE );
			}
                    | name_seq type_spec
                        {
			  add_names( $1, $2 );
			}
                      typed_names
                    | /* empty */
                    ;

name_seq            : name
                        {
			  $$ = new std::vector<const std::string*>( 1, $1 );
			}
                    | name_seq name
                        {
			  $$ = $1;
			  $$->push_back( $2 );
			}
                    ;

type_spec           : HYPHEN
                        {
			  require_typing();
			}
                      type
                       {
			 $$ = $3;
		       }
                    ;

type                : object
                        {
			  $$ = OBJECT_TYPE;
			}
                    | type_name
                        {
			  $$ = make_type( $1 );
			}
                    | LEFTPAR either types RIGHTPAR
                        {
			  $$ = make_type( *$3 );
			}
                    ;

types               : object
                        {
			  $$ = new TypeSet;
			}
                    | type_name
                        {
			  $$ = new TypeSet;
			  $$->insert( make_type( $1 ) );
			}
                    | types object
                        {
			  $$ = $1;
			}
                    | types type_name
                        {
			  $$ = $1;
			  $$->insert( make_type( $2 ) );
			}
                    ;

function_type       : number
                    ;

define              : DEFINE { delete $1; }
                    ;

domain              : DOMAIN_TOKEN { delete $1; }
                    ;

problem             : PROBLEM { delete $1; }
                    ;

when                : WHEN { delete $1; }
                    ;

not                 : NOT { delete $1; }
                    ;

and                 : AND { delete $1; }
                    ;

or                  : OR { delete $1; }
                    ;

imply               : IMPLY { delete $1; }
                    ;

exists              : EXISTS { delete $1; }
                    ;

forall              : FORALL { delete $1; }
                    ;

probabilistic       : PROBABILISTIC { delete $1; }
                    ;

assign              : ASSIGN { delete $1; }
                    ;

scale_up            : SCALE_UP { delete $1; }
                    ;

scale_down          : SCALE_DOWN { delete $1; }
                    ;

increase            : INCREASE { delete $1; }
                    ;

decrease            : DECREASE { delete $1; }
                    ;

minimize            : MINIMIZE { delete $1; }
                    ;

maximize            : MAXIMIZE { delete $1; }
                    ;

number              : NUMBER_TOKEN { delete $1; }
                    ;

object              : OBJECT_TOKEN { delete $1; }
                    ;

either              : EITHER { delete $1; }
                    ;

type_name           : DEFINE
                    | DOMAIN_TOKEN
                    | PROBLEM
                    | EITHER
                    | MINIMIZE
                    | MAXIMIZE
                    | NAME
                    ;

predicate           : type_name
                    | OBJECT_TOKEN
                    | NUMBER_TOKEN
                    ;

function            : name
                    ;

name                : DEFINE
                    | DOMAIN_TOKEN
                    | PROBLEM
                    | NUMBER_TOKEN
                    | OBJECT_TOKEN
                    | EITHER
                    | WHEN
                    | NOT
                    | AND
                    | OR
                    | IMPLY
                    | EXISTS
                    | FORALL
                    | PROBABILISTIC
                    | ASSIGN
                    | SCALE_UP
                    | SCALE_DOWN
                    | INCREASE
                    | DECREASE
                    | MINIMIZE
                    | MAXIMIZE
                    | NAME
                    ;

variable            : VARIABLE
                    ;

%%

void
yyerror( const char *s )
{
  std::string str( s );
  yyerror( str );
}

static void
yyerror( const std::string &s )
{
  std::cout << "<parser>:" << current_file << ':' << line_number << ": " << s
	    << std::endl;
  success = false;
}

static void
yywarning( const std::string &s )
{
  if( gpt::warning_level > 0 )
    {
      std::cout << "<parser>:" << current_file << ':' << line_number << ": " << s
		<< std::endl;
      if( gpt::warning_level > 1 )
	success = false;
    }
}

static void
make_domain( const std::string *name )
{
  domain = new Domain( *name );
  domains[*name] = domain;
  requirements = &domain->requirements;
  problem = NULL;
  delete name;
}

static void
make_problem( const std::string *name, const std::string *domain_name )
{
  std::map<std::string,Domain*>::const_iterator di = domains.find( *domain_name );
  if( di != domains.end() )
    domain = (*di).second;
  else
    {
      domain = new Domain( *domain_name );
      domains[*domain_name] = domain;
      yyerror( "undeclared domain `" + *domain_name + "' used" );
    }

  // analyze and simplify domain
  if( gpt::domain_analysis )
    domain->analyze_and_simplify();

  requirements = new Requirements( domain->requirements );
  problem = problem_t::allocate( *name, *domain );
  if( requirements->rewards )
    {
      const Application& reward_appl =
	Application::make_application( reward_function, TermList() );
      const Assignment* reward_assignment =
	new Assignment( Assignment::ASSIGN_OP, reward_appl, *new Value(0) );
      problem->add_init_effect( *new AssignmentEffect( *reward_assignment ) );
    }
  delete name;
  delete domain_name;
}

static void
require_typing( void )
{
  if( !requirements->typing )
    {
      yywarning( "assuming `:typing' requirement" );
      requirements->typing = true;
    }
}

static void
require_fluents( void )
{
  if( !requirements->fluents )
    {
      yywarning( "assuming `:fluents' requirement" );
      requirements->fluents = true;
    }
}

static void
require_disjunction( void )
{
  if( !requirements->disjunctive_preconditions )
    {
      yywarning( "assuming `:disjunctive-preconditions' requirement" );
      requirements->disjunctive_preconditions = true;
    }
}

static void
require_conditional_effects( void )
{
  if( !requirements->conditional_effects )
    {
      yywarning( "assuming `:conditional-effects' requirement" );
      requirements->conditional_effects = true;
    }
}

static Type
make_type( const std::string *name )
{
  std::pair<Type,bool> t = domain->types().find_type( *name );
  if( !t.second )
    {
      t.first = domain->types().add_type( *name );
      if( name_kind != TYPE_KIND )
	yywarning( "implicit declaration of type `" + *name + "'" );
    }
  delete name;
  return( t.first );
}

static Type
make_type( const TypeSet& types )
{
  return( domain->types().add_type( types ) );
}

static Term
make_term( const std::string *name )
{
  if( (*name)[0] == '?' )
    {
      std::pair<Variable,bool> v = context.find( *name );
      if( !v.second )
	{
	  if( problem != NULL )
	    v.first = problem->terms().add_variable( OBJECT_TYPE );
	  else
	    v.first = domain->terms().add_variable( OBJECT_TYPE );
	  context.insert( *name, v.first );
	  yyerror( "free variable `" + *name + "' used" );
	}
      delete name;
      return( v.first );
    }
  else
    {
      TermTable& terms = (problem!=NULL?problem->terms():domain->terms());
      const PredicateTable& predicates = domain->predicates();
      std::pair<Object,bool> o = terms.find_object( *name );
      if( !o.second )
	{
	  size_t n = term_parameters.size();
	  if( parsing_atom && (predicates.arity( atom_predicate ) > n) )
	    o.first = terms.add_object( *name, predicates.parameter( atom_predicate, n ) );
	  else
	    o.first = terms.add_object( *name, OBJECT_TYPE );
	}
      delete name;
      return( o.first );
    }
}

static void
make_predicate( const std::string *name )
{
  repeated_predicate = false;
  std::pair<Predicate,bool> p = domain->predicates().find_predicate( *name );
  if( !p.second )
    p.first = domain->predicates().add_predicate( *name );
  else
    {
      repeated_predicate = true;
      yywarning( "ignoring repeated declaration of predicate `" + *name + "'" );
    }
  predicate = p.first;
  parsing_predicate = true;
  delete name;
}

static void
make_function( const std::string *name )
{
  repeated_function = false;
  std::pair<Function,bool> f = domain->functions().find_function( *name );
  if( !f.second )
    f.first = domain->functions().add_function( *name );
  else
    {
      repeated_function = true;
      if( requirements->rewards && (f.first == reward_function) )
	yywarning( "ignoring declaration of reserved function `reward'" );
      else
	yywarning( "ignoring repeated declaration of function `" + *name + "'" );
    }
  function = f.first;
  parsing_function = true;
  delete name;
}

static void
make_action( const std::string *name )
{
  context.push_frame();
  action = new ActionSchema( *name );
  delete name;
}

static void
add_action( void )
{
  context.pop_frame();
  if( domain->find_action( action->name() ) == NULL )
    domain->add_action( *action );
  else
    {
      yywarning( "ignoring repeated declaration of action `" + action->name() + "'" );
      delete action;
    }
  action = NULL;
}

static void
prepare_forall_effect( void )
{
  if( !requirements->conditional_effects )
    {
      yywarning( "assuming `:conditional-effects' requirement" );
      requirements->conditional_effects = true;
    }
  context.push_frame();
  quantified.push_back( NULL_TERM );
}

static const Effect*
make_forall_effect( const Effect& effect )
{
  context.pop_frame();
  QuantifiedEffect* qeffect = new QuantifiedEffect( effect );
  Effect::unregister_use( &effect );
  size_t n = quantified.size() - 1;
  size_t m = n;
  for( ; is_variable( quantified[n] ); --n );
  for( size_t i = n + 1; i <= m; ++i )
    qeffect->add_parameter( quantified[i] );
  quantified.resize( n );
  return( qeffect );
}

static void
add_effect_outcome( ProbabilisticEffect& peffect, const Rational* p, const Effect& effect )
{
  if( !requirements->probabilistic_effects )
    {
      yywarning( "assuming `:probabilistic-effects' requirement" );
      requirements->probabilistic_effects = true;
    }
  if( (*p < 0) || (*p > 1) )
    yyerror( "outcome probability needs to be in the interval [0,1]" );
  if( !peffect.add_outcome( *p, effect ) )
    yyerror( "effect outcome probabilities add up to more than 1" );
  delete p;
}

static const Effect* 
make_add_effect( const Atom& atom )
{
  domain->predicates().make_dynamic( atom.predicate() );
  const Effect *result = new AddEffect( atom );
  StateFormula::unregister_use( &atom );
  return( result );
}

static const Effect* 
make_delete_effect( const Atom& atom )
{
  domain->predicates().make_dynamic( atom.predicate() );
  const Effect *result = new DeleteEffect( atom );
  StateFormula::unregister_use( &atom );
  return( result );
}

static const Effect*
make_assignment_effect( unsigned oper,
			const Application& application,
			const Expression& expr )
{
  if( requirements->rewards && (application.function() == reward_function) )
    {
      if( ((oper != Assignment::INCREASE_OP) && (oper != Assignment::DECREASE_OP)) ||
	  (typeid(expr) != typeid(Value)) )
	yyerror( "only constant reward increments/decrements allowed" );
    }
  else
    require_fluents();
  effect_fluent = false;
  domain->functions().make_dynamic( application.function() );
  const Assignment& assignment = *new Assignment( oper, application, expr );
  //Expression::unregister_use( &expr );
  const Effect* result = new AssignmentEffect( assignment );
  return( result );
}

static void
add_names( const std::vector<const std::string*>* names, Type type )
{
  for( std::vector<const std::string*>::const_iterator si = names->begin(); si != names->end(); si++ )
    {
      const std::string *s = *si;
      if( name_kind == TYPE_KIND )
	{
	  if( *s == OBJECT_NAME )
	    yywarning( "ignoring declaration of reserved type `object'" );
	  else if( *s == NUMBER_NAME )
	    yywarning( "ignoring declaration of reserved type `number'" );
	  else
	    {
	      std::pair<Type,bool> t = domain->types().find_type( *s );
	       if( !t.second )
		 t.first = domain->types().add_type( *s );
	       if( !domain->types().add_supertype( t.first, type ) )
		 yyerror( "cyclic type hierarchy" );
	    }
	}
      else if( name_kind == CONSTANT_KIND )
	{
	  std::pair<Object,bool> o = domain->terms().find_object( *s );
	  if( !o.second )
	    domain->terms().add_object( *s, type );
	  else
	    {
	      TypeSet components;
	      domain->types().components( components, domain->terms().type( o.first ) );
	      components.insert( type );
	      domain->terms().set_type( o.first, make_type( components ) );
	    }
	}
      else // name_kind == OBJECT_KIND 
	{ 
	  if( domain->terms().find_object( *s ).second )
	    yywarning( "ignoring declaration of object `" + *s + "' previously declared as constant" );
	  else
	    {
	      std::pair<Object,bool> o = problem->terms().find_object( *s );
	      if( !o.second )
		problem->terms().add_object( *s, type );
	      else
		{
		  TypeSet components;
		  domain->types().components( components, problem->terms().type( o.first ) );
		  components.insert( type );
		  problem->terms().set_type( o.first, make_type( components ) );
		}
	    }
	}
      delete s;
    }
  delete names;
}

static void
add_variables( const std::vector<const std::string*>* names, Type type )
{
  for( std::vector<const std::string*>::const_iterator si = names->begin(); si != names->end(); ++si )
    {
      const std::string *s = *si;
      if( parsing_predicate )
	{
	  if( !repeated_predicate )
	    domain->predicates().add_parameter( predicate, type );
	}
      else if( parsing_function )
	{
	  if( !repeated_function )
	    domain->functions().add_parameter( function, type );
	}
      else
	{
	  if( context.shallow_find(*s).second )
	    yyerror( "repetition of parameter `" + *s + "'" );
	  else if( context.find(*s).second )
	    yywarning( "shadowing parameter `" + *s + "'" );
	  
	  Variable var;
	  if( problem != NULL )
	    var = problem->terms().add_variable( type );
	  else
	    var = domain->terms().add_variable( type );

	  context.insert( *s, var );
	  if( !quantified.empty() )
	    quantified.push_back( var );
	  else
	    action->add_parameter( var );
	}
      delete s;
    }
  delete names;
}

static void
prepare_atom( const std::string *name )
{
  std::pair<Predicate,bool> p = domain->predicates().find_predicate( *name );
  if( !p.second )
    {
      atom_predicate = domain->predicates().add_predicate( *name );
      undeclared_atom_predicate = true;
      if( problem != NULL )
	yywarning( "undeclared predicate `" + *name + "' used" );
      else
	yywarning( "implicit declaration of predicate `" + *name + "'" );
    }
  else
    {
      atom_predicate = p.first;
      undeclared_atom_predicate = false;
    }
  term_parameters.clear();
  parsing_atom = true;
  delete name;
}

static void 
prepare_application( const std::string *name )
{
  std::pair<Function,bool> f = domain->functions().find_function( *name );
  if( !f.second )
    {
      appl_function = domain->functions().add_function( *name );
      undeclared_appl_function = true;
      if( problem != NULL )
	yywarning( "undeclared function `" + *name + "' used" );
      else
	yywarning( "implicit declaration of function `" + *name + "'" );
    }
  else
    {
      appl_function = f.first;
      undeclared_appl_function = false;
    }

  if( requirements->rewards && (f.first == reward_function) )
    {
      if( false && !effect_fluent )
	yyerror( "reserved function `reward' not allowed here" );
    }
  else
    require_fluents();
  term_parameters.clear();
  parsing_application = true;
  delete name;
}

static void 
add_term( const std::string *name )
{
  Term term = make_term( name );
  const TermTable& terms = (problem!=NULL?problem->terms():domain->terms());
  if( parsing_atom )
    {
      PredicateTable& predicates = domain->predicates();
      size_t n = term_parameters.size();
      if( undeclared_atom_predicate )
	{
	  predicates.add_parameter( atom_predicate, terms.type( term ) );
	}
      else if( (predicates.arity( atom_predicate ) > n) &&
	       !domain->types().subtype( terms.type(term),
					 predicates.parameter(atom_predicate,n) ) )
	yyerror( "type mismatch" );
    }
  else if( parsing_application )
    {
      FunctionTable& functions = domain->functions();
      size_t n = term_parameters.size();
      if( undeclared_appl_function )
	functions.add_parameter( appl_function, terms.type( term ) );
      else if( (functions.arity( appl_function ) > n) &&
	       !domain->types().subtype( terms.type(term),
					 functions.parameter(appl_function,n) ) )
	yyerror( "type mismatch" );
    }
  term_parameters.push_back( term );
}

static const Atom* 
make_atom( void )
{
  size_t n = term_parameters.size();
  if( domain->predicates().arity( atom_predicate ) < n )
    yyerror( "too many parameters passed to predicate `"
	     + domain->predicates().name(atom_predicate) + "'" );
  else if( domain->predicates().arity( atom_predicate ) > n )
    yyerror( "too few parameters passed to predicate `"
	     + domain->predicates().name(atom_predicate) + "'" );
  parsing_atom = false;
  return( &Atom::make_atom( atom_predicate, term_parameters ) );
}

static const Application* 
make_application( void )
{
  size_t n = term_parameters.size();
  if( domain->functions().arity( appl_function ) < n )
    yyerror( "too many parameters passed to function `"
	     + domain->functions().name(appl_function) + "'" );
  else if( domain->functions().arity( appl_function ) > n )
    yyerror( "too few parameters passed to function `"
	     + domain->functions().name(appl_function) + "'" );
  parsing_application = false;
  return( &Application::make_application( appl_function, term_parameters ) );
}

static const Expression* 
make_subtraction( const Expression& term, const Expression* opt_term )
{
  if( opt_term != NULL )
    {
      const Expression *result = new Subtraction( term, *opt_term );
      Expression::unregister_use( &term );
      Expression::unregister_use( opt_term );
      return( result );
    }
  else
    {
      opt_term = new Value( 0 );
      const Expression *result = new Subtraction( *opt_term, term );
      Expression::unregister_use( &term );
      Expression::unregister_use( opt_term );
      return( result );
    }
}

static void 
make_eq_name( const std::string *name )
{
  std::pair<Function,bool> f = domain->functions().find_function( *name );
  if( f.second )
    {
      prepare_application( name );
      eq_expr = make_application();
    }
  else
    {
      // assume this is a term.
      eq_term = make_term( name );
      eq_expr = NULL;
    }
}

static const StateFormula* 
make_equality( void )
{
  if( !requirements->equality )
    {
      yywarning( "assuming `:equality' requirement" );
      requirements->equality = true;
    }
  if( (first_eq_expr != NULL) && (eq_expr != NULL) )
    {
      return( new Comparison( Comparison::EQ_CMP, *first_eq_expr, *eq_expr ) );
    }
  else if( (first_eq_expr == NULL) && (eq_expr == NULL) )
    {
      const TermTable& terms = (problem!=NULL?problem->terms():domain->terms());
      if( domain->types().subtype( terms.type(first_eq_term), terms.type(eq_term) ) ||
	  domain->types().subtype( terms.type(eq_term), terms.type(first_eq_term) ) )
	return( new Equality( first_eq_term, eq_term ) );
      else
	{
	  StateFormula::register_use( &StateFormula::FALSE );
	  return( &StateFormula::FALSE );
	}
    }
  else
    {
      yyerror( "comparison of term and numeric expression" );
      StateFormula::register_use( &StateFormula::FALSE );
      return( &StateFormula::FALSE );
    }
}

static const StateFormula* 
make_negation( const StateFormula& negand )
{
  if( (typeid(negand) == typeid(Atom)) && !requirements->negative_preconditions )
    {
      yywarning( "assuming `:negative-preconditions' requirement" );
      requirements->negative_preconditions = true;
    }
  else if( (typeid(negand) != typeid(Equality)) && (typeid(negand) != typeid(Comparison)) )
    {
      require_disjunction();
    }
  return( &Negation::make_negation( negand ) );
}

static const StateFormula* 
make_implication( const StateFormula& f1, const StateFormula& f2 )
{
  require_disjunction();
  Disjunction* disj = new Disjunction();
  disj->add_disjunct( Negation::make_negation( f1 ) );
  disj->add_disjunct( f2 );
  return( disj );
}

static void 
prepare_exists( void )
{
  if( !requirements->existential_preconditions )
    {
      yywarning( "assuming `:existential-preconditions' requirement" );
      requirements->existential_preconditions = true;
    }
  context.push_frame();
  quantified.push_back( NULL_TERM );
}

static void 
prepare_forall( void )
{
  if( !requirements->universal_preconditions )
    {
      yywarning( "assuming `:universal-preconditions' requirement" );
      requirements->universal_preconditions = true;
    }
  context.push_frame();
  quantified.push_back( NULL_TERM );
}

static const StateFormula* 
make_exists( const StateFormula& body )
{
  context.pop_frame();
  size_t n, m = quantified.size() - 1;
  for( n = m; is_variable( quantified[n] ); --n );
  if( n < m )
    {
      Exists* exists = new Exists;
      for( size_t i = n + 1; i <= m; ++i )
	exists->add_parameter( quantified[i] );
      exists->set_body( body );
      StateFormula::unregister_use( &body );
      quantified.resize( n );
      return( exists );
    }
  else
    {
      quantified.pop_back();
      return( &body );
    }
}

static const StateFormula* 
make_forall( const StateFormula& body )
{
  context.pop_frame();
  size_t n, m = quantified.size() - 1;
  for( n = m; is_variable( quantified[n] ); --n );
  if( n < m )
    {
      Forall* forall = new Forall;
      for( size_t i = n + 1; i <= m; ++i )
	forall->add_parameter( quantified[i] );
      forall->set_body( body );
      StateFormula::unregister_use( &body );
      quantified.resize( n );
      return( forall );
    }
  else
    {
      quantified.pop_back();
      return( &body );
    }
}
