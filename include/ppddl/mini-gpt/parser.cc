/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 1 "parser.y" /* yacc.c:339  */

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
extern std::string current_file;
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

#line 221 "y.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "y.tab.h".  */
#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    DEFINE = 258,
    DOMAIN_TOKEN = 259,
    PROBLEM = 260,
    REQUIREMENTS = 261,
    TYPES = 262,
    CONSTANTS = 263,
    PREDICATES = 264,
    FUNCTIONS = 265,
    STRIPS = 266,
    TYPING = 267,
    NEGATIVE_PRECONDITIONS = 268,
    DISJUNCTIVE_PRECONDITIONS = 269,
    EQUALITY = 270,
    EXISTENTIAL_PRECONDITIONS = 271,
    UNIVERSAL_PRECONDITIONS = 272,
    QUANTIFIED_PRECONDITIONS = 273,
    CONDITIONAL_EFFECTS = 274,
    FLUENTS = 275,
    ADL = 276,
    DURATIVE_ACTIONS = 277,
    DURATION_INEQUALITIES = 278,
    CONTINUOUS_EFFECTS = 279,
    PROBABILISTIC_EFFECTS = 280,
    REWARDS = 281,
    MDP = 282,
    ACTION = 283,
    PARAMETERS = 284,
    PRECONDITION = 285,
    EFFECT = 286,
    PDOMAIN = 287,
    OBJECTS = 288,
    INIT = 289,
    GOAL = 290,
    GOAL_REWARD = 291,
    METRIC = 292,
    EXPECTED_COST = 293,
    GOAL_PROBABILITY = 294,
    WHEN = 295,
    NOT = 296,
    AND = 297,
    OR = 298,
    IMPLY = 299,
    EXISTS = 300,
    FORALL = 301,
    PROBABILISTIC = 302,
    ASSIGN = 303,
    SCALE_UP = 304,
    SCALE_DOWN = 305,
    INCREASE = 306,
    DECREASE = 307,
    MINIMIZE = 308,
    MAXIMIZE = 309,
    NUMBER_TOKEN = 310,
    OBJECT_TOKEN = 311,
    EITHER = 312,
    LT = 313,
    LE = 314,
    GE = 315,
    GT = 316,
    NAME = 317,
    VARIABLE = 318,
    NUMBER = 319,
    ILLEGAL_TOKEN = 320,
    PLUS = 321,
    HYPHEN = 322,
    STAR = 323,
    SLASH = 324,
    LEFTPAR = 325,
    RIGHTPAR = 326,
    LEFTBRACE = 327,
    RIGHTBRACE = 328,
    EQUAL = 329,
    ORBIT = 330,
    SYSTEM = 331
  };
#endif
/* Tokens.  */
#define DEFINE 258
#define DOMAIN_TOKEN 259
#define PROBLEM 260
#define REQUIREMENTS 261
#define TYPES 262
#define CONSTANTS 263
#define PREDICATES 264
#define FUNCTIONS 265
#define STRIPS 266
#define TYPING 267
#define NEGATIVE_PRECONDITIONS 268
#define DISJUNCTIVE_PRECONDITIONS 269
#define EQUALITY 270
#define EXISTENTIAL_PRECONDITIONS 271
#define UNIVERSAL_PRECONDITIONS 272
#define QUANTIFIED_PRECONDITIONS 273
#define CONDITIONAL_EFFECTS 274
#define FLUENTS 275
#define ADL 276
#define DURATIVE_ACTIONS 277
#define DURATION_INEQUALITIES 278
#define CONTINUOUS_EFFECTS 279
#define PROBABILISTIC_EFFECTS 280
#define REWARDS 281
#define MDP 282
#define ACTION 283
#define PARAMETERS 284
#define PRECONDITION 285
#define EFFECT 286
#define PDOMAIN 287
#define OBJECTS 288
#define INIT 289
#define GOAL 290
#define GOAL_REWARD 291
#define METRIC 292
#define EXPECTED_COST 293
#define GOAL_PROBABILITY 294
#define WHEN 295
#define NOT 296
#define AND 297
#define OR 298
#define IMPLY 299
#define EXISTS 300
#define FORALL 301
#define PROBABILISTIC 302
#define ASSIGN 303
#define SCALE_UP 304
#define SCALE_DOWN 305
#define INCREASE 306
#define DECREASE 307
#define MINIMIZE 308
#define MAXIMIZE 309
#define NUMBER_TOKEN 310
#define OBJECT_TOKEN 311
#define EITHER 312
#define LT 313
#define LE 314
#define GE 315
#define GT 316
#define NAME 317
#define VARIABLE 318
#define NUMBER 319
#define ILLEGAL_TOKEN 320
#define PLUS 321
#define HYPHEN 322
#define STAR 323
#define SLASH 324
#define LEFTPAR 325
#define RIGHTPAR 326
#define LEFTBRACE 327
#define RIGHTBRACE 328
#define EQUAL 329
#define ORBIT 330
#define SYSTEM 331

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 175 "parser.y" /* yacc.c:355  */

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

#line 433 "y.tab.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 448 "y.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1118

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  77
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  146
/* YYNRULES -- Number of rules.  */
#define YYNRULES  308
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  491

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   331

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   221,   221,   221,   233,   234,   235,   239,   238,   245,
     246,   247,   248,   251,   252,   253,   256,   257,   258,   259,
     260,   261,   262,   265,   266,   267,   268,   269,   272,   273,
     274,   275,   276,   279,   280,   281,   282,   283,   286,   287,
     288,   291,   292,   293,   296,   297,   298,   301,   302,   305,
     308,   311,   312,   315,   319,   323,   327,   331,   335,   339,
     343,   347,   351,   355,   359,   363,   367,   371,   377,   382,
     393,   392,   404,   403,   413,   417,   416,   423,   424,   428,
     427,   437,   438,   439,   442,   443,   447,   446,   454,   453,
     464,   463,   473,   474,   477,   478,   481,   482,   485,   492,
     499,   500,   505,   504,   513,   512,   520,   526,   532,   537,
     542,   549,   552,   556,   561,   560,   570,   574,   578,   582,
     586,   594,   592,   603,   604,   607,   608,   611,   612,   615,
     616,   619,   620,   623,   624,   627,   628,   631,   632,   636,
     635,   645,   648,   649,   652,   657,   662,   669,   674,   681,
     682,   688,   694,   699,   703,   709,   716,   723,   726,   731,
     736,   739,   740,   743,   750,   755,   762,   763,   766,   772,
     778,   784,   789,   794,   793,   803,   802,   812,   816,   821,
     820,   828,   833,   832,   841,   840,   850,   855,   860,   865,
     871,   870,   878,   886,   885,   893,   900,   901,   902,   903,
     906,   911,   917,   923,   929,   935,   941,   948,   947,   958,
     957,   966,   965,   976,   975,   986,   985,   994,   998,  1005,
    1007,  1013,  1012,  1020,  1028,  1027,  1035,  1042,  1046,  1050,
    1053,  1057,  1060,  1065,  1064,  1069,  1072,  1076,  1083,  1088,
    1087,  1092,  1095,  1099,  1107,  1106,  1116,  1120,  1124,  1130,
    1134,  1139,  1143,  1150,  1153,  1156,  1159,  1162,  1165,  1168,
    1171,  1174,  1177,  1180,  1183,  1186,  1189,  1192,  1195,  1198,
    1201,  1204,  1207,  1210,  1213,  1216,  1217,  1218,  1219,  1220,
    1221,  1222,  1225,  1226,  1227,  1230,  1233,  1234,  1235,  1236,
    1237,  1238,  1239,  1240,  1241,  1242,  1243,  1244,  1245,  1246,
    1247,  1248,  1249,  1250,  1251,  1252,  1253,  1254,  1257
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "DEFINE", "DOMAIN_TOKEN", "PROBLEM",
  "REQUIREMENTS", "TYPES", "CONSTANTS", "PREDICATES", "FUNCTIONS",
  "STRIPS", "TYPING", "NEGATIVE_PRECONDITIONS",
  "DISJUNCTIVE_PRECONDITIONS", "EQUALITY", "EXISTENTIAL_PRECONDITIONS",
  "UNIVERSAL_PRECONDITIONS", "QUANTIFIED_PRECONDITIONS",
  "CONDITIONAL_EFFECTS", "FLUENTS", "ADL", "DURATIVE_ACTIONS",
  "DURATION_INEQUALITIES", "CONTINUOUS_EFFECTS", "PROBABILISTIC_EFFECTS",
  "REWARDS", "MDP", "ACTION", "PARAMETERS", "PRECONDITION", "EFFECT",
  "PDOMAIN", "OBJECTS", "INIT", "GOAL", "GOAL_REWARD", "METRIC",
  "EXPECTED_COST", "GOAL_PROBABILITY", "WHEN", "NOT", "AND", "OR", "IMPLY",
  "EXISTS", "FORALL", "PROBABILISTIC", "ASSIGN", "SCALE_UP", "SCALE_DOWN",
  "INCREASE", "DECREASE", "MINIMIZE", "MAXIMIZE", "NUMBER_TOKEN",
  "OBJECT_TOKEN", "EITHER", "LT", "LE", "GE", "GT", "NAME", "VARIABLE",
  "NUMBER", "ILLEGAL_TOKEN", "PLUS", "HYPHEN", "STAR", "SLASH", "LEFTPAR",
  "RIGHTPAR", "LEFTBRACE", "RIGHTBRACE", "EQUAL", "ORBIT", "SYSTEM",
  "$accept", "file", "$@1", "domains_and_problems", "domain_def", "$@2",
  "domain_body", "domain_body2", "domain_body3", "domain_body4",
  "domain_body5", "domain_body6", "domain_body7", "domain_body8",
  "domain_body9", "structure_defs", "structure_def", "require_def",
  "require_keys", "require_key", "types_def", "$@3", "constants_def",
  "$@4", "predicates_def", "functions_def", "$@5", "predicate_decls",
  "predicate_decl", "$@6", "function_decls", "function_decl_seq",
  "function_type_spec", "$@7", "function_decl", "$@8", "action_def", "$@9",
  "parameters", "action_body", "action_body2", "precondition", "effect",
  "eff_formula", "$@10", "$@11", "eff_formulas", "prob_effs",
  "probability", "p_effect", "$@12", "assign_op", "problem_def", "$@13",
  "problem_body", "problem_body2", "problem_body3", "problem_body4",
  "problem_body5", "problem_body6", "problem_body7", "problem_body8",
  "object_decl", "$@14", "init", "init_elements", "init_element",
  "prob_inits", "simple_init", "one_inits", "one_init", "value", "goal",
  "goal_reward", "metric_spec", "orbit_decls", "orbit_decl", "atoms",
  "system_decls", "system_decl", "orbit_set", "orbits", "formula", "$@15",
  "$@16", "$@17", "$@18", "$@19", "conjuncts", "disjuncts",
  "atomic_term_formula", "$@20", "atomic_name_formula", "$@21",
  "binary_comp", "f_exp", "term_or_f_exp", "$@22", "$@23", "$@24", "$@25",
  "$@26", "opt_f_exp", "f_head", "$@27", "ground_f_head", "$@28", "terms",
  "names", "variables", "$@29", "variable_seq", "typed_names", "$@30",
  "name_seq", "type_spec", "$@31", "type", "types", "function_type",
  "define", "domain", "problem", "when", "not", "and", "or", "imply",
  "exists", "forall", "probabilistic", "assign", "scale_up", "scale_down",
  "increase", "decrease", "minimize", "maximize", "number", "object",
  "either", "type_name", "predicate", "function", "name", "variable", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331
};
# endif

#define YYPACT_NINF -348

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-348)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -348,    20,  -348,  -348,   -45,    47,  -348,  -348,  -348,   -10,
     125,  -348,  -348,  1011,  1011,  -348,  -348,  -348,  -348,  -348,
    -348,  -348,  -348,  -348,  -348,  -348,  -348,  -348,  -348,  -348,
    -348,  -348,  -348,  -348,  -348,  -348,  -348,    -6,    11,  -348,
      66,    69,   118,   284,    81,  -348,  -348,    84,  -348,    97,
     106,   108,   110,   119,  -348,  1011,  1091,  -348,  -348,  -348,
    -348,  1011,  -348,   128,  -348,   218,  -348,   206,  -348,   149,
    -348,    84,   124,   132,    35,  -348,    84,   124,   136,    78,
    -348,    84,   132,   136,   139,  -348,  -348,  -348,  -348,  -348,
    -348,  -348,  -348,  -348,  -348,  -348,  -348,  -348,  -348,  -348,
    -348,  -348,  1013,  -348,  1011,  1011,   113,   142,  -348,    86,
    -348,    84,    84,    19,  -348,    84,    84,  -348,    28,  -348,
      84,    84,  -348,  -348,  -348,  -348,  -348,   147,   956,  -348,
     148,   358,  -348,  -348,  1011,   162,    51,  -348,   180,    84,
      84,    84,   166,  -348,  -348,  -348,  -348,  -348,  -348,  -348,
    -348,  -348,  -348,  -348,  -348,  -348,  -348,  -348,  -348,  -348,
    -348,  -348,  -348,   142,  -348,   168,   155,     4,   170,   171,
    -348,  -348,  -348,  -348,  -348,  -348,  -348,   173,   174,   175,
     181,   182,   182,  -348,   188,  -348,   825,  1011,   187,   187,
     210,  -348,   187,   763,   794,   190,  -348,   248,  -348,  -348,
    -348,   763,   221,   138,   224,   225,    -2,  -348,  -348,    65,
    -348,    90,  -348,    41,  -348,    56,  -348,   122,  -348,  -348,
    -348,   212,  -348,  -348,   238,  -348,  -348,  -348,  -348,  -348,
     228,   -41,  -348,   233,  -348,  -348,  -348,   234,   273,  -348,
    -348,  -348,  1039,  -348,  -348,  -348,  -348,  -348,  1011,   200,
     236,  -348,   239,  -348,  -348,   275,   646,   818,   250,  -348,
     370,  -348,  -348,  -348,  -348,  -348,  -348,  -348,  -348,  -348,
    -348,  -348,  -348,  -348,  -348,  -348,   614,  -348,   763,  -348,
    -348,   763,  -348,  -348,  -348,  -348,  -348,  -348,  -348,  -348,
    -348,  -348,  -348,  -348,   836,  -348,  -348,   247,  -348,  -348,
    -348,  -348,  -348,   249,    14,  -348,  -348,  -348,  -348,  -348,
    -348,   252,   253,  1011,   254,  -348,   358,   227,  -348,  -348,
     250,   582,  -348,  -348,   187,  -348,   896,  -348,  -348,  -348,
     677,   265,   512,  -348,   763,   267,   276,  -348,   708,   763,
     358,   274,   522,   278,  -348,    -9,   794,  -348,   739,   247,
    -348,  -348,  -348,  -348,  -348,  -348,  -348,    72,   250,  -348,
    -348,  -348,  -348,  -348,  -348,  -348,  -348,  -348,   614,  -348,
     926,   677,  -348,  -348,  -348,  -348,   544,   279,   187,   187,
     338,   677,   794,  -348,  -348,  -348,   187,  -348,   794,  -348,
    1011,   285,  -348,   854,    33,  -348,  -348,  -348,  -348,   280,
     677,   677,   677,   677,  -348,   281,   677,   677,   677,   677,
     282,  -348,  -348,  -348,   283,   286,  -348,  -348,  -348,   295,
     297,   300,  -348,  -348,   301,   107,  -348,  -348,  -348,  -348,
     854,   457,   393,  -348,   677,   677,   677,   677,   425,  -348,
     677,   677,   677,   677,  -348,   763,   763,  -348,  -348,   794,
    -348,  -348,   739,  -348,  -348,  -348,  -348,  -348,   305,  -348,
     306,   331,   333,  -348,   334,   335,   336,   339,   345,   346,
     350,   489,   221,   554,  -348,  -348,  -348,  -348,  -348,  -348,
    -348,  -348,  -348,  -348,  -348,  -348,   351,   167,  -348,  -348,
    -348
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     6,     1,     3,     0,     4,     5,   254,     0,
       0,   255,   256,     0,     0,   286,   287,   288,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   289,   290,   291,   307,     0,     0,     7,
       0,    12,     0,     0,     0,    11,    15,    22,    47,     9,
      13,    16,    17,    18,    49,     0,     0,    70,    72,    78,
      75,     0,     8,     0,    48,     0,    10,     0,    14,     0,
      19,    27,    23,    24,     0,    20,    32,    28,    29,     0,
      21,    37,    33,    34,     0,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,     0,    51,   241,   241,     0,    83,    90,     0,
      25,    40,    38,     0,    26,    43,    41,    30,     0,    31,
      46,    44,    35,    36,   121,    50,    52,     0,   238,   242,
       0,     0,    74,    77,     0,     0,    81,    84,    93,    39,
      42,    45,   138,    71,   244,   239,   243,    73,   275,   276,
     277,   279,   280,   284,   283,   278,   281,   282,    79,    88,
     285,    76,    86,    83,    85,     0,    97,     0,   138,     0,
     124,   126,   128,   130,   132,   134,   136,   138,   138,   138,
     138,   138,   138,   162,   137,   167,     0,   241,   235,   235,
       0,    82,   235,     0,     0,     0,    95,    97,    96,   139,
     143,     0,     0,     0,     0,     0,     0,   123,   122,     0,
     125,     0,   127,     0,   129,     0,   131,     0,   133,   135,
     161,     0,   166,   273,     0,   245,   246,   247,   240,   308,
       0,   232,   236,     0,   272,    87,   253,     0,     0,    98,
     172,   192,     0,    99,   100,   112,    91,    94,   241,     0,
       0,   155,     0,   270,   271,     0,     0,     0,     0,   274,
       0,    80,   233,   237,    89,    92,   258,   259,   260,   261,
     262,   263,   196,   197,   198,   199,     0,   175,     0,   187,
     179,     0,   182,   184,   190,   257,   264,   265,   266,   267,
     268,   269,   114,   104,     0,   108,   102,     0,   116,   117,
     118,   119,   120,     0,     0,   141,   142,   144,   195,   156,
     157,     0,     0,     0,     0,   223,     0,     0,   165,   171,
       0,     0,   249,   250,   235,   206,     0,   173,   217,   218,
       0,     0,     0,   189,     0,     0,     0,   229,     0,     0,
       0,     0,     0,     0,   111,     0,     0,   140,     0,     0,
     193,   159,   158,   221,   160,   163,   164,     0,     0,   248,
     251,   252,   234,   207,   209,   211,   213,   215,     0,   200,
       0,     0,   205,   177,   178,   186,     0,     0,   235,   235,
       0,     0,     0,   113,   101,   107,   235,   106,     0,   109,
       0,     0,   226,     0,     0,   231,   229,   170,   169,     0,
       0,     0,     0,     0,   229,     0,     0,     0,     0,     0,
       0,   180,   188,   181,     0,     0,   191,   227,   228,     0,
       0,     0,   110,   224,     0,     0,   147,   149,   153,   146,
       0,     0,     0,   168,     0,   220,     0,     0,     0,   174,
       0,   220,     0,     0,   176,     0,     0,   115,   105,     0,
     231,   145,     0,   152,   148,   194,   230,   222,     0,   219,
       0,     0,     0,   216,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   208,   210,   212,   214,   201,   202,
     203,   204,   183,   185,   103,   225,     0,     0,   150,   151,
     154
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -348,  -348,  -348,  -348,  -348,  -348,  -348,   307,   381,  -348,
    -348,  -348,   374,   371,   369,   287,   -17,   312,  -348,   355,
    -348,  -348,    55,  -348,    -7,    96,  -348,  -348,  -348,  -348,
     296,  -348,  -348,  -348,   327,  -348,  -348,  -348,  -348,  -348,
     288,  -348,  -348,  -330,  -348,  -348,  -348,  -348,  -322,  -348,
    -348,  -348,  -348,  -348,  -348,   315,   309,   311,   316,   304,
     310,   308,  -348,  -348,  -348,  -348,  -348,  -348,    88,  -348,
      48,    50,  -348,  -348,  -348,  -348,   341,  -348,  -348,   340,
    -305,  -348,  -197,  -348,  -348,  -348,  -348,  -348,  -348,  -348,
    -189,  -348,  -228,  -348,  -348,  -134,   152,  -348,  -348,  -348,
    -348,  -348,   109,  -243,  -348,   100,  -348,  -347,   103,  -178,
    -348,  -348,   -97,  -348,  -348,   323,  -348,  -348,  -348,  -348,
    -348,  -348,  -348,  -348,   313,  -235,  -348,  -348,  -348,   314,
     257,  -348,  -348,  -348,  -348,  -348,  -348,  -348,  -348,  -236,
    -348,  -177,  -129,  -131,   -13,  -225
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,     6,    41,    44,    45,    46,    70,
      75,    80,   110,   114,   119,    47,    48,    49,   102,   103,
      50,   104,    51,   105,    52,    53,   107,   106,   133,   188,
     135,   136,   163,   190,   137,   189,    54,   138,   166,   195,
     196,   197,   198,   243,   343,   339,   342,   345,   346,   244,
     338,   292,     7,   142,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   248,   178,   249,   306,   394,   426,   473,
     427,   252,   179,   180,   181,   182,   183,   317,   184,   185,
     320,   357,   239,   368,   330,   333,   335,   336,   332,   376,
     240,   337,   428,   395,   277,   459,   327,   400,   401,   402,
     403,   404,   460,   372,   396,   391,   450,   380,   431,   230,
     324,   231,   127,   187,   128,   145,   186,   225,   321,   235,
       9,    13,    14,   293,   278,   279,   280,   281,   282,   283,
     297,   298,   299,   300,   301,   302,   255,   256,   236,   226,
     260,   157,   241,   315,   160,   232
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint16 yytable[] =
{
      37,    38,   158,   159,   250,   245,   263,   295,   130,   227,
      56,   233,   385,   314,   237,   358,   389,   148,   149,   150,
       3,   307,   229,   388,   322,     5,   144,   393,    59,   318,
      64,   199,   200,   201,   202,   203,    58,   199,   200,   201,
     202,   203,    84,    58,    72,    60,    83,    61,   108,   432,
       8,   329,   420,   399,    64,   344,    61,   438,   422,    64,
      10,   286,   387,    61,    64,    39,   116,   151,   152,   153,
     154,   155,   430,   204,   205,   116,   156,   202,   203,   204,
     205,   331,    40,   323,   334,   360,    58,    59,   348,   356,
     228,   129,   129,   203,    64,   381,    60,   344,    64,   200,
     201,   202,   203,    64,   429,   341,    61,    77,    82,   284,
     148,   149,   150,   284,    61,   146,   204,   205,   162,   470,
     308,   134,    64,    64,    64,   201,   202,   203,   308,    11,
      12,   204,   205,   121,   397,   375,    42,   377,   121,    43,
     204,   205,   382,   329,   361,   398,   362,    73,    78,   267,
      55,   303,    62,   245,    63,   418,    61,   245,    59,    60,
     151,   152,   153,   154,   155,   204,   205,    65,   112,   156,
     148,   149,   150,   112,   129,   350,    67,    61,    69,   412,
      74,   452,   353,   131,   132,   193,   194,   350,   308,    79,
     453,   253,   254,   245,   109,   367,   371,   204,   205,   245,
     414,   415,   113,   148,   149,   150,   118,   418,   421,   165,
     124,   284,   134,   418,    58,    59,    60,   392,   143,   147,
     151,   152,   153,   154,   155,    57,    58,    59,    60,   156,
     148,   149,   150,   161,    61,   129,   167,   410,   192,   353,
     206,   452,   208,   209,   211,   213,    61,   419,   468,   469,
     229,   215,   217,   151,   152,   153,   154,   155,   221,   423,
     245,   246,   156,   328,   308,   234,   434,   435,   436,   437,
     304,   305,   440,   441,   442,   443,   148,   149,   150,   194,
     151,   152,   153,   154,   155,   251,   257,   258,   205,   156,
      56,    57,    58,    59,    60,   259,   350,   316,   355,   261,
     458,   308,   461,   462,   264,   265,   464,   309,   466,   467,
     310,   344,    61,   311,   266,   267,   268,   269,   270,   271,
     347,   392,   319,   351,   352,   354,   151,   152,   153,   154,
     155,   272,   273,   274,   275,   156,   373,   378,    71,    76,
      81,    15,    16,    17,   308,   383,   379,   276,   386,   424,
     413,   433,   439,   444,   445,   328,    66,   446,   350,   111,
     115,   148,   149,   150,   111,   120,   447,   417,   448,   115,
     120,   449,   451,   148,   149,   150,   474,   475,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    15,    16,    17,   139,
      36,   229,   476,   140,   477,   478,   479,   480,   141,   416,
     481,   151,   152,   153,   154,   155,   482,   483,   456,   417,
     156,   484,   490,   151,   152,   417,   223,   155,    15,    16,
      17,    68,   156,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,   117,   123,   122,   168,    36,   229,   126,   456,   191,
      15,    16,    17,   164,   457,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,   207,   216,   247,   210,    36,   229,   212,
     219,   218,    15,    16,    17,   214,   463,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,   148,   149,   150,   454,    36,
     405,   489,   486,   220,   222,   148,   149,   150,   455,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,   148,   149,   150,
     465,    36,   472,   471,   262,   294,   296,   148,   149,   150,
     485,   349,     0,     0,     0,   151,   152,   153,   154,   155,
       0,     0,     0,     0,   156,   151,   152,   153,   154,   155,
       0,     0,   238,   374,   156,   148,   149,   150,     0,     0,
       0,     0,   242,   384,     0,     0,     0,   151,   152,   153,
     154,   155,     0,     0,     0,     0,   156,   151,   152,   153,
     154,   155,     0,     0,   238,   411,   156,    15,    16,    17,
       0,     0,     0,     0,   487,   488,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   151,   152,     0,   223,   155,
       0,     0,     0,     0,   156,     0,     0,     0,     0,    15,
      16,    17,     0,   359,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,     0,     0,     0,     0,    36,   229,   325,     0,
      15,    16,    17,     0,   326,   312,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,     0,     0,     0,     0,    36,     0,
       0,    15,    16,    17,     0,     0,   313,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,     0,     0,     0,     0,    36,
       0,   369,    15,    16,    17,     0,     0,   370,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,   148,   149,   150,     0,
      36,     0,     0,     0,     0,     0,     0,     0,   313,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,   148,   149,   150,
       0,    36,     0,     0,     0,     0,     0,     0,     0,   390,
       0,     0,     0,     0,     0,     0,   151,   152,   153,   154,
     155,   148,   149,   150,     0,   156,     0,     0,   148,   149,
     150,     0,     0,   238,     0,     0,     0,     0,     0,   148,
     149,   150,     0,     0,     0,     0,     0,   151,   152,   153,
     154,   155,     0,     0,     0,     0,   156,   148,   149,   150,
       0,     0,     0,     0,   242,     0,     0,     0,     0,     0,
       0,   151,   152,   153,   154,   155,     0,     0,   151,   152,
     156,   223,   155,     0,     0,     0,     0,   156,   316,   151,
     152,   153,   154,   155,     0,   224,     0,     0,   156,    15,
      16,    17,     0,     0,     0,     0,   340,   151,   152,   153,
     154,   155,     0,     0,     0,     0,   156,     0,     0,     0,
       0,     0,     0,     0,   425,     0,     0,     0,     0,    15,
      16,    17,     0,     0,     0,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,     0,     0,     0,     0,    36,    15,
      16,    17,   363,   364,   365,   366,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,     0,     0,     0,     0,    36,     0,
       0,     0,   406,   407,   408,   409,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    15,    16,    17,     0,    36,     0,
       0,     0,     0,   144,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,     0,   148,   149,   150,     0,     0,     0,     0,     0,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,     0,
       0,     0,     0,    36,     0,     0,     0,     0,     0,   285,
     266,   267,     0,     0,   125,   271,   286,   287,   288,   289,
     290,   291,   151,   152,   153,   154,   155,     0,     0,     0,
       0,   156,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101
};

static const yytype_int16 yycheck[] =
{
      13,    14,   131,   134,   201,   194,   231,   242,   105,   186,
       6,   189,   342,   256,   192,   320,   346,     3,     4,     5,
       0,   249,    63,   345,   260,    70,    67,   349,     9,   257,
      47,    33,    34,    35,    36,    37,     8,    33,    34,    35,
      36,    37,    55,     8,    51,    10,    53,    28,    61,   396,
       3,   276,   382,   358,    71,    64,    28,   404,   388,    76,
      70,    47,    71,    28,    81,    71,    73,    53,    54,    55,
      56,    57,   394,    75,    76,    82,    62,    36,    37,    75,
      76,   278,    71,   260,   281,   321,     8,     9,    74,   317,
     187,   104,   105,    37,   111,   338,    10,    64,   115,    34,
      35,    36,    37,   120,    71,   294,    28,    52,    53,   238,
       3,     4,     5,   242,    28,   128,    75,    76,    67,   449,
     249,    70,   139,   140,   141,    35,    36,    37,   257,     4,
       5,    75,    76,    78,    62,   332,    70,   334,    83,    70,
      75,    76,   339,   368,   321,    73,   324,    51,    52,    42,
      32,   248,    71,   342,    70,   380,    28,   346,     9,    10,
      53,    54,    55,    56,    57,    75,    76,    70,    72,    62,
       3,     4,     5,    77,   187,   304,    70,    28,    70,   376,
      70,    74,   313,    70,    71,    30,    31,   316,   317,    70,
     425,    53,    54,   382,    70,   326,   330,    75,    76,   388,
     378,   379,    70,     3,     4,     5,    70,   432,   386,    29,
      71,   340,    70,   438,     8,     9,    10,   348,    71,    71,
      53,    54,    55,    56,    57,     7,     8,     9,    10,    62,
       3,     4,     5,    71,    28,   248,    70,   371,    70,   370,
      70,    74,    71,    70,    70,    70,    28,   381,   445,   446,
      63,    70,    70,    53,    54,    55,    56,    57,    70,   390,
     449,    71,    62,   276,   393,    55,   400,   401,   402,   403,
      70,    71,   406,   407,   408,   409,     3,     4,     5,    31,
      53,    54,    55,    56,    57,    64,    62,    62,    76,    62,
       6,     7,     8,     9,    10,    57,   425,    70,    71,    71,
     434,   430,   436,   437,    71,    71,   440,    71,   442,   443,
      71,    64,    28,    38,    41,    42,    43,    44,    45,    46,
      71,   452,    72,    71,    71,    71,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    71,    70,    51,    52,
      53,     3,     4,     5,   473,    71,    70,    74,    70,    64,
      71,    71,    71,    71,    71,   368,    49,    71,   487,    72,
      73,     3,     4,     5,    77,    78,    71,   380,    71,    82,
      83,    71,    71,     3,     4,     5,    71,    71,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,     3,     4,     5,   112,
      62,    63,    71,   116,    71,    71,    71,    71,   121,    71,
      71,    53,    54,    55,    56,    57,    71,    71,   431,   432,
      62,    71,    71,    53,    54,   438,    56,    57,     3,     4,
       5,    50,    62,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    77,    83,    82,   142,    62,    63,   102,   471,   163,
       3,     4,     5,   136,    71,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,   168,   180,   197,   177,    62,    63,   178,
     182,   181,     3,     4,     5,   179,    71,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,     3,     4,     5,   430,    62,
     368,   473,   472,   182,   184,     3,     4,     5,    71,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,     3,     4,     5,
     441,    62,   452,   450,   231,   242,   242,     3,     4,     5,
      71,   304,    -1,    -1,    -1,    53,    54,    55,    56,    57,
      -1,    -1,    -1,    -1,    62,    53,    54,    55,    56,    57,
      -1,    -1,    70,    71,    62,     3,     4,     5,    -1,    -1,
      -1,    -1,    70,    71,    -1,    -1,    -1,    53,    54,    55,
      56,    57,    -1,    -1,    -1,    -1,    62,    53,    54,    55,
      56,    57,    -1,    -1,    70,    71,    62,     3,     4,     5,
      -1,    -1,    -1,    -1,    70,    71,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    53,    54,    -1,    56,    57,
      -1,    -1,    -1,    -1,    62,    -1,    -1,    -1,    -1,     3,
       4,     5,    -1,    71,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    -1,    -1,    -1,    -1,    62,    63,    64,    -1,
       3,     4,     5,    -1,    70,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    -1,    -1,    -1,    -1,    62,    -1,
      -1,     3,     4,     5,    -1,    -1,    70,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    -1,    -1,    -1,    -1,    62,
      -1,    64,     3,     4,     5,    -1,    -1,    70,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,     3,     4,     5,    -1,
      62,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,     3,     4,     5,
      -1,    62,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    53,    54,    55,    56,
      57,     3,     4,     5,    -1,    62,    -1,    -1,     3,     4,
       5,    -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,     3,
       4,     5,    -1,    -1,    -1,    -1,    -1,    53,    54,    55,
      56,    57,    -1,    -1,    -1,    -1,    62,     3,     4,     5,
      -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,
      -1,    53,    54,    55,    56,    57,    -1,    -1,    53,    54,
      62,    56,    57,    -1,    -1,    -1,    -1,    62,    70,    53,
      54,    55,    56,    57,    -1,    70,    -1,    -1,    62,     3,
       4,     5,    -1,    -1,    -1,    -1,    70,    53,    54,    55,
      56,    57,    -1,    -1,    -1,    -1,    62,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,     3,
       4,     5,    -1,    -1,    -1,    -1,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    -1,    -1,    -1,    -1,    62,     3,
       4,     5,    66,    67,    68,    69,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    -1,    -1,    -1,    -1,    62,    -1,
      -1,    -1,    66,    67,    68,    69,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,     3,     4,     5,    -1,    62,    -1,
      -1,    -1,    -1,    67,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    -1,     3,     4,     5,    -1,    -1,    -1,    -1,    -1,
      -1,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    -1,
      -1,    -1,    -1,    62,    -1,    -1,    -1,    -1,    -1,    40,
      41,    42,    -1,    -1,    71,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    -1,    -1,    -1,
      -1,    62,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    78,    79,     0,    80,    70,    81,   129,     3,   197,
      70,     4,     5,   198,   199,     3,     4,     5,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    62,   221,   221,    71,
      71,    82,    70,    70,    83,    84,    85,    92,    93,    94,
      97,    99,   101,   102,   113,    32,     6,     7,     8,     9,
      10,    28,    71,    70,    93,    70,    84,    70,    85,    70,
      86,    92,   101,   102,    70,    87,    92,    99,   102,    70,
      88,    92,    99,   101,   221,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    95,    96,    98,   100,   104,   103,   221,    70,
      89,    92,   102,    70,    90,    92,   101,    89,    70,    91,
      92,    99,    90,    91,    71,    71,    96,   189,   191,   221,
     189,    70,    71,   105,    70,   107,   108,   111,   114,    92,
      92,    92,   130,    71,    67,   192,   221,    71,     3,     4,
       5,    53,    54,    55,    56,    57,    62,   218,   219,   220,
     221,    71,    67,   109,   111,    29,   115,    70,    94,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   141,   149,
     150,   151,   152,   153,   155,   156,   193,   190,   106,   112,
     110,   107,    70,    30,    31,   116,   117,   118,   119,    33,
      34,    35,    36,    37,    75,    76,    70,   132,    71,    70,
     133,    70,   134,    70,   135,    70,   136,    70,   137,   138,
     153,    70,   156,    56,    70,   194,   216,   218,   189,    63,
     186,   188,   222,   186,    55,   196,   215,   186,    70,   159,
     167,   219,    70,   120,   126,   167,    71,   117,   140,   142,
     159,    64,   148,    53,    54,   213,   214,    62,    62,    57,
     217,    71,   192,   222,    71,    71,    41,    42,    43,    44,
      45,    46,    58,    59,    60,    61,    74,   171,   201,   202,
     203,   204,   205,   206,   219,    40,    47,    48,    49,    50,
      51,    52,   128,   200,   201,   202,   206,   207,   208,   209,
     210,   211,   212,   189,    70,    71,   143,   169,   219,    71,
      71,    38,    39,    70,   180,   220,    70,   154,   169,    72,
     157,   195,   216,   218,   187,    64,    70,   173,   221,   222,
     161,   159,   165,   162,   159,   163,   164,   168,   127,   122,
      70,   167,   123,   121,    64,   124,   125,    71,    74,   207,
     219,    71,    71,   220,    71,    71,   169,   158,   157,    71,
     216,   218,   186,    66,    67,    68,    69,   220,   160,    64,
      70,   172,   180,    71,    71,   159,   166,   159,    70,    70,
     184,   180,   159,    71,    71,   120,    70,    71,   125,   120,
      70,   182,   220,   125,   144,   170,   181,    62,    73,   157,
     174,   175,   176,   177,   178,   173,    66,    67,    68,    69,
     172,    71,   159,    71,   186,   186,    71,   221,   222,   172,
     120,   186,   120,   220,    64,    70,   145,   147,   169,    71,
     125,   185,   184,    71,   172,   172,   172,   172,   184,    71,
     172,   172,   172,   172,    71,    71,    71,    71,    71,    71,
     183,    71,    74,   202,   145,    71,   221,    71,   172,   172,
     179,   172,   172,    71,   172,   179,   172,   172,   159,   159,
     120,   185,   182,   146,    71,    71,    71,    71,    71,    71,
      71,    71,    71,    71,    71,    71,   148,    70,    71,   147,
      71
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    77,    79,    78,    80,    80,    80,    82,    81,    83,
      83,    83,    83,    84,    84,    84,    85,    85,    85,    85,
      85,    85,    85,    86,    86,    86,    86,    86,    87,    87,
      87,    87,    87,    88,    88,    88,    88,    88,    89,    89,
      89,    90,    90,    90,    91,    91,    91,    92,    92,    93,
      94,    95,    95,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      98,    97,   100,    99,   101,   103,   102,   104,   104,   106,
     105,   107,   107,   107,   108,   108,   110,   109,   112,   111,
     114,   113,   115,   115,   116,   116,   117,   117,   118,   119,
     120,   120,   121,   120,   122,   120,   120,   123,   123,   124,
     124,   125,   126,   126,   127,   126,   128,   128,   128,   128,
     128,   130,   129,   131,   131,   132,   132,   133,   133,   134,
     134,   135,   135,   136,   136,   137,   137,   138,   138,   140,
     139,   141,   142,   142,   143,   143,   143,   144,   144,   145,
     145,   146,   146,   147,   147,   148,   149,   150,   151,   151,
     151,   152,   152,   153,   154,   154,   155,   155,   156,   157,
     158,   158,   159,   160,   159,   161,   159,   159,   159,   162,
     159,   159,   163,   159,   164,   159,   165,   165,   166,   166,
     168,   167,   167,   170,   169,   169,   171,   171,   171,   171,
     172,   172,   172,   172,   172,   172,   173,   174,   173,   175,
     173,   176,   173,   177,   173,   178,   173,   173,   173,   179,
     179,   181,   180,   180,   183,   182,   182,   184,   184,   184,
     185,   185,   186,   187,   186,   186,   188,   188,   189,   190,
     189,   189,   191,   191,   193,   192,   194,   194,   194,   195,
     195,   195,   195,   196,   197,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   218,   218,   218,   218,
     218,   218,   219,   219,   219,   220,   221,   221,   221,   221,
     221,   221,   221,   221,   221,   221,   221,   221,   221,   221,
     221,   221,   221,   221,   221,   221,   221,   221,   222
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     2,     0,     0,     9,     1,
       2,     1,     0,     1,     2,     1,     1,     1,     1,     2,
       2,     2,     1,     1,     1,     2,     2,     1,     1,     1,
       2,     2,     1,     1,     1,     2,     2,     1,     1,     2,
       1,     1,     2,     1,     1,     2,     1,     1,     2,     1,
       4,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     5,     0,     5,     4,     0,     5,     2,     0,     0,
       5,     1,     3,     0,     1,     2,     0,     3,     0,     5,
       0,     7,     4,     0,     2,     1,     1,     0,     2,     2,
       1,     4,     0,     8,     0,     6,     4,     2,     0,     2,
       3,     1,     1,     4,     0,     6,     1,     1,     1,     1,
       1,     0,    13,     2,     1,     2,     1,     2,     1,     2,
       1,     2,     1,     2,     1,     2,     1,     1,     0,     0,
       5,     4,     2,     0,     1,     5,     4,     2,     3,     1,
       4,     2,     0,     1,     5,     1,     4,     4,     5,     5,
       5,     2,     1,     5,     2,     1,     2,     1,     7,     3,
       2,     0,     1,     0,     6,     0,     6,     4,     4,     0,
       5,     5,     0,     8,     0,     8,     2,     0,     2,     0,
       0,     5,     1,     0,     5,     1,     1,     1,     1,     1,
       1,     5,     5,     5,     5,     1,     1,     0,     6,     0,
       6,     0,     6,     0,     6,     0,     5,     1,     1,     1,
       0,     0,     5,     1,     0,     5,     1,     2,     2,     0,
       2,     0,     1,     0,     4,     0,     1,     2,     1,     0,
       4,     0,     1,     2,     0,     3,     1,     1,     4,     1,
       1,     2,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 221 "parser.y" /* yacc.c:1646  */
    {
                          success = true;
			  line_number = 1;
			}
#line 2058 "y.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 226 "parser.y" /* yacc.c:1646  */
    {
			  if( !success )
			    YYERROR;
			}
#line 2067 "y.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 239 "parser.y" /* yacc.c:1646  */
    {
			  make_domain( (yyvsp[-1].str) );
			}
#line 2075 "y.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 316 "parser.y" /* yacc.c:1646  */
    {
			  requirements->strips = true;
			}
#line 2083 "y.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 320 "parser.y" /* yacc.c:1646  */
    {
			  requirements->typing = true;
			}
#line 2091 "y.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 324 "parser.y" /* yacc.c:1646  */
    {
			  requirements->negative_preconditions = true;
			}
#line 2099 "y.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 328 "parser.y" /* yacc.c:1646  */
    {
			  requirements->disjunctive_preconditions = true;
			}
#line 2107 "y.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 332 "parser.y" /* yacc.c:1646  */
    {
			  requirements->equality = true;
			}
#line 2115 "y.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 336 "parser.y" /* yacc.c:1646  */
    {
			  requirements->existential_preconditions = true;
			}
#line 2123 "y.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 340 "parser.y" /* yacc.c:1646  */
    {
			  requirements->universal_preconditions = true;
			}
#line 2131 "y.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 344 "parser.y" /* yacc.c:1646  */
    {
			  requirements->quantified_preconditions();
			}
#line 2139 "y.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 348 "parser.y" /* yacc.c:1646  */
    {
			  requirements->conditional_effects = true;
			}
#line 2147 "y.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 352 "parser.y" /* yacc.c:1646  */
    {
			  requirements->fluents = true;
			}
#line 2155 "y.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 356 "parser.y" /* yacc.c:1646  */
    {
			  requirements->adl();
			}
#line 2163 "y.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 360 "parser.y" /* yacc.c:1646  */
    {
			  throw Exception( "`:durative-actions' not supported" );
			}
#line 2171 "y.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 364 "parser.y" /* yacc.c:1646  */
    {
			  throw Exception( "`:duration-inequalities' not supported" );
			}
#line 2179 "y.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 368 "parser.y" /* yacc.c:1646  */
    {
			  throw Exception( "`:continuous-effects' not supported" );
			}
#line 2187 "y.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 372 "parser.y" /* yacc.c:1646  */
    {
			  requirements->probabilistic_effects = true;
			  goal_prob_function =
			    domain->functions().add_function( "goal-probability" );
			}
#line 2197 "y.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 378 "parser.y" /* yacc.c:1646  */
    {
			  requirements->rewards = true;
			  reward_function = domain->functions().add_function( "reward" );
			}
#line 2206 "y.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 383 "parser.y" /* yacc.c:1646  */
    {
			  requirements->probabilistic_effects = true;
			  requirements->rewards = true;
			  goal_prob_function =
			    domain->functions().add_function( "goal-probability" );
			  reward_function = domain->functions().add_function( "reward" );
			}
#line 2218 "y.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 393 "parser.y" /* yacc.c:1646  */
    {
			  require_typing();
			  name_kind = TYPE_KIND;
			}
#line 2227 "y.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 398 "parser.y" /* yacc.c:1646  */
    {
			  name_kind = VOID_KIND;
			}
#line 2235 "y.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 404 "parser.y" /* yacc.c:1646  */
    {
			  name_kind = CONSTANT_KIND;
			}
#line 2243 "y.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 408 "parser.y" /* yacc.c:1646  */
    {
			  name_kind = VOID_KIND;
			}
#line 2251 "y.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 417 "parser.y" /* yacc.c:1646  */
    {
			  require_fluents();
			}
#line 2259 "y.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 428 "parser.y" /* yacc.c:1646  */
    {
			  make_predicate( (yyvsp[0].str) );
			}
#line 2267 "y.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 432 "parser.y" /* yacc.c:1646  */
    {
			  parsing_predicate = false;
			}
#line 2275 "y.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 447 "parser.y" /* yacc.c:1646  */
    {
			  require_typing();
			}
#line 2283 "y.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 454 "parser.y" /* yacc.c:1646  */
    {
			  make_function( (yyvsp[0].str) );
			}
#line 2291 "y.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 458 "parser.y" /* yacc.c:1646  */
    {
			  parsing_function = false;
			}
#line 2299 "y.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 464 "parser.y" /* yacc.c:1646  */
    {
			  make_action( (yyvsp[0].str) );
			}
#line 2307 "y.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 468 "parser.y" /* yacc.c:1646  */
    {
			  add_action();
			}
#line 2315 "y.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 486 "parser.y" /* yacc.c:1646  */
    {
			  action->set_precondition( *(yyvsp[0].formula) );
			  StateFormula::unregister_use( (yyvsp[0].formula) );
			}
#line 2324 "y.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 493 "parser.y" /* yacc.c:1646  */
    {
			  action->set_effect( *(yyvsp[0].effect) );
			  Effect::unregister_use( (yyvsp[0].effect) );
			}
#line 2333 "y.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 501 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.effect) = (yyvsp[-1].ceffect);
			}
#line 2341 "y.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 505 "parser.y" /* yacc.c:1646  */
    {
			  prepare_forall_effect();
			}
#line 2349 "y.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 509 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.effect) = make_forall_effect( *(yyvsp[-1].effect) );
			}
#line 2357 "y.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 513 "parser.y" /* yacc.c:1646  */
    {
			  require_conditional_effects();
			}
#line 2365 "y.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 517 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.effect) = &ConditionalEffect::make( *(yyvsp[-2].formula), *(yyvsp[-1].effect) );
			}
#line 2373 "y.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 521 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.effect) = (yyvsp[-1].peffect);
			}
#line 2381 "y.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 527 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.ceffect) = (yyvsp[-1].ceffect);
			  (yyval.ceffect)->add_conjunct( *(yyvsp[0].effect) );
			}
#line 2390 "y.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 532 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.ceffect) = new ConjunctiveEffect;
			}
#line 2398 "y.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 538 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.peffect) = new ProbabilisticEffect;
			  add_effect_outcome( *(yyval.peffect), (yyvsp[-1].num), *(yyvsp[0].effect) );
			}
#line 2407 "y.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 543 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.peffect) = (yyvsp[-2].peffect);
			  add_effect_outcome( *(yyval.peffect), (yyvsp[-1].num), *(yyvsp[0].effect) );
			}
#line 2416 "y.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 553 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.effect) = make_add_effect( *(yyvsp[0].atom) );
			}
#line 2424 "y.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 557 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.effect) = make_delete_effect( *(yyvsp[-1].atom) );
			}
#line 2432 "y.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 561 "parser.y" /* yacc.c:1646  */
    {
			  effect_fluent = true;
			}
#line 2440 "y.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 565 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.effect) = make_assignment_effect( (yyvsp[-4].setop), *(yyvsp[-2].appl), *(yyvsp[-1].expr) );
			}
#line 2448 "y.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 571 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.setop) = Assignment::ASSIGN_OP;
			}
#line 2456 "y.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 575 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.setop) = Assignment::SCALE_UP_OP;
			}
#line 2464 "y.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 579 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.setop) = Assignment::SCALE_DOWN_OP;
			}
#line 2472 "y.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 583 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.setop) = Assignment::INCREASE_OP;
			}
#line 2480 "y.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 587 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.setop) = Assignment::DECREASE_OP;
			}
#line 2488 "y.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 594 "parser.y" /* yacc.c:1646  */
    {
			  make_problem( (yyvsp[-5].str), (yyvsp[-1].str) );
			}
#line 2496 "y.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 598 "parser.y" /* yacc.c:1646  */
    {
			  delete requirements;
			}
#line 2504 "y.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 636 "parser.y" /* yacc.c:1646  */
    {
			  name_kind = OBJECT_KIND;
			}
#line 2512 "y.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 640 "parser.y" /* yacc.c:1646  */
    {
			  name_kind = VOID_KIND;
			}
#line 2520 "y.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 653 "parser.y" /* yacc.c:1646  */
    {
			  problem->add_init_atom( *(yyvsp[0].atom) );
			  StateFormula::unregister_use( (yyvsp[0].atom) );
			}
#line 2529 "y.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 658 "parser.y" /* yacc.c:1646  */
    {
			  problem->add_init_fluent( *(yyvsp[-2].appl), *(yyvsp[-1].num) );
			  delete (yyvsp[-1].num);
			}
#line 2538 "y.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 663 "parser.y" /* yacc.c:1646  */
    {
			  problem->add_init_effect( *(yyvsp[-1].peffect) );
			  Effect::unregister_use( (yyvsp[-1].peffect) );
			}
#line 2547 "y.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 670 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.peffect) = new ProbabilisticEffect;
			  add_effect_outcome( *(yyval.peffect), (yyvsp[-1].num), *(yyvsp[0].effect) );
			}
#line 2556 "y.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 675 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.peffect) = (yyvsp[-2].peffect);
			  add_effect_outcome( *(yyval.peffect), (yyvsp[-1].num), *(yyvsp[0].effect) );
			}
#line 2565 "y.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 683 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.effect) = (yyvsp[-1].ceffect);
			}
#line 2573 "y.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 689 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.ceffect) = (yyvsp[-1].ceffect);
			  (yyval.ceffect)->add_conjunct( *(yyvsp[0].effect) );
			}
#line 2582 "y.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 694 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.ceffect) = new ConjunctiveEffect;
			}
#line 2590 "y.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 700 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.effect) = make_add_effect( *(yyvsp[0].atom) );
			}
#line 2598 "y.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 704 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.effect) = make_assignment_effect( Assignment::ASSIGN_OP, *(yyvsp[-2].appl), *(yyvsp[-1].expr) );
			}
#line 2606 "y.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 710 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.expr) = new Value( *(yyvsp[0].num) );
			  delete (yyvsp[0].num);
			}
#line 2615 "y.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 717 "parser.y" /* yacc.c:1646  */
    {
			  problem->set_goal( *(yyvsp[-1].formula) );
			  StateFormula::unregister_use( (yyvsp[-1].formula) );
			}
#line 2624 "y.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 727 "parser.y" /* yacc.c:1646  */
    {
			  /* don't care about rewards (blai) */
			  /* problem->set_metric( problem_t::MAXIMIZE_GOAL_PROBABILITY ); */
			}
#line 2633 "y.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 732 "parser.y" /* yacc.c:1646  */
    {
			  /* don't care about rewards (blai) */
			  /* problem->set_metric( problem_t::MINIMIZE_EXPECTED_COST ); */
			}
#line 2642 "y.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 744 "parser.y" /* yacc.c:1646  */
    {
			  problem->add_orbit( (yyvsp[-2].str), (yyvsp[-1].atoms) );
			  delete (yyvsp[-2].str);
			}
#line 2651 "y.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 751 "parser.y" /* yacc.c:1646  */
    {
			  (yyvsp[-1].atoms)->push_back( (yyvsp[0].atom) );
			  (yyval.atoms) = (yyvsp[-1].atoms);
			}
#line 2660 "y.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 756 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.atoms) = new std::vector<const Atom*>;
			  (yyval.atoms)->push_back( (yyvsp[0].atom) );
			}
#line 2669 "y.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 767 "parser.y" /* yacc.c:1646  */
    {
			  problem->add_system( (yyvsp[-4].str), (yyvsp[-3].strs), (yyvsp[-2].strs), (yyvsp[-1].strs) );
			}
#line 2677 "y.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 773 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.strs) = (yyvsp[-1].strs);
			}
#line 2685 "y.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 779 "parser.y" /* yacc.c:1646  */
    {
			  (yyvsp[-1].strs)->push_back( (yyvsp[0].str) );
			  (yyval.strs) = (yyvsp[-1].strs);
			}
#line 2694 "y.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 784 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.strs) = new std::vector<const std::string*>;
			}
#line 2702 "y.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 790 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.formula) = (yyvsp[0].atom);
			}
#line 2710 "y.tab.c" /* yacc.c:1646  */
    break;

  case 173:
#line 794 "parser.y" /* yacc.c:1646  */
    {
			  first_eq_term = eq_term;
			  first_eq_expr = eq_expr;
			}
#line 2719 "y.tab.c" /* yacc.c:1646  */
    break;

  case 174:
#line 799 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.formula) = make_equality();
			}
#line 2727 "y.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 803 "parser.y" /* yacc.c:1646  */
    {
			  require_fluents();
			}
#line 2735 "y.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 807 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.formula) = new Comparison( (yyvsp[-4].comp), *(yyvsp[-2].expr), *(yyvsp[-1].expr) );
			  Expression::unregister_use( (yyvsp[-2].expr) );
			  Expression::unregister_use( (yyvsp[-1].expr) );
			}
#line 2745 "y.tab.c" /* yacc.c:1646  */
    break;

  case 177:
#line 813 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.formula) = make_negation( *(yyvsp[-1].formula) );
			}
#line 2753 "y.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 817 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.formula) = (yyvsp[-1].conj);
			}
#line 2761 "y.tab.c" /* yacc.c:1646  */
    break;

  case 179:
#line 821 "parser.y" /* yacc.c:1646  */
    {
			  require_disjunction();
			}
#line 2769 "y.tab.c" /* yacc.c:1646  */
    break;

  case 180:
#line 825 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.formula) = (yyvsp[-1].disj);
			}
#line 2777 "y.tab.c" /* yacc.c:1646  */
    break;

  case 181:
#line 829 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.formula) = make_implication( *(yyvsp[-2].formula), *(yyvsp[-1].formula) );
			}
#line 2785 "y.tab.c" /* yacc.c:1646  */
    break;

  case 182:
#line 833 "parser.y" /* yacc.c:1646  */
    {
			  prepare_exists();
			}
#line 2793 "y.tab.c" /* yacc.c:1646  */
    break;

  case 183:
#line 837 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.formula) = make_exists( *(yyvsp[-1].formula) );
			}
#line 2801 "y.tab.c" /* yacc.c:1646  */
    break;

  case 184:
#line 841 "parser.y" /* yacc.c:1646  */
    {
			  prepare_forall();
			}
#line 2809 "y.tab.c" /* yacc.c:1646  */
    break;

  case 185:
#line 845 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.formula) = make_forall( *(yyvsp[-1].formula) );
			}
#line 2817 "y.tab.c" /* yacc.c:1646  */
    break;

  case 186:
#line 851 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.conj)->add_conjunct( *(yyvsp[0].formula) );
			}
#line 2825 "y.tab.c" /* yacc.c:1646  */
    break;

  case 187:
#line 855 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.conj) = new Conjunction;
			}
#line 2833 "y.tab.c" /* yacc.c:1646  */
    break;

  case 188:
#line 861 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.disj)->add_disjunct( *(yyvsp[0].formula) );
			}
#line 2841 "y.tab.c" /* yacc.c:1646  */
    break;

  case 189:
#line 865 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.disj) = new Disjunction;
			}
#line 2849 "y.tab.c" /* yacc.c:1646  */
    break;

  case 190:
#line 871 "parser.y" /* yacc.c:1646  */
    {
			  prepare_atom( (yyvsp[0].str) );
			}
#line 2857 "y.tab.c" /* yacc.c:1646  */
    break;

  case 191:
#line 875 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.atom) = make_atom();
			}
#line 2865 "y.tab.c" /* yacc.c:1646  */
    break;

  case 192:
#line 879 "parser.y" /* yacc.c:1646  */
    {
			  prepare_atom( (yyvsp[0].str) );
			  (yyval.atom) = make_atom();
			}
#line 2874 "y.tab.c" /* yacc.c:1646  */
    break;

  case 193:
#line 886 "parser.y" /* yacc.c:1646  */
    {
			  prepare_atom( (yyvsp[0].str) );
			}
#line 2882 "y.tab.c" /* yacc.c:1646  */
    break;

  case 194:
#line 890 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.atom) = make_atom();
			}
#line 2890 "y.tab.c" /* yacc.c:1646  */
    break;

  case 195:
#line 894 "parser.y" /* yacc.c:1646  */
    {
			  prepare_atom( (yyvsp[0].str) );
			  (yyval.atom) = make_atom();
			}
#line 2899 "y.tab.c" /* yacc.c:1646  */
    break;

  case 196:
#line 900 "parser.y" /* yacc.c:1646  */
    { (yyval.comp) = Comparison::LT_CMP; }
#line 2905 "y.tab.c" /* yacc.c:1646  */
    break;

  case 197:
#line 901 "parser.y" /* yacc.c:1646  */
    { (yyval.comp) = Comparison::LE_CMP; }
#line 2911 "y.tab.c" /* yacc.c:1646  */
    break;

  case 198:
#line 902 "parser.y" /* yacc.c:1646  */
    { (yyval.comp) = Comparison::GE_CMP; }
#line 2917 "y.tab.c" /* yacc.c:1646  */
    break;

  case 199:
#line 903 "parser.y" /* yacc.c:1646  */
    { (yyval.comp) = Comparison::GT_CMP; }
#line 2923 "y.tab.c" /* yacc.c:1646  */
    break;

  case 200:
#line 907 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.expr) = new Value( *(yyvsp[0].num) );
			  delete (yyvsp[0].num);
			}
#line 2932 "y.tab.c" /* yacc.c:1646  */
    break;

  case 201:
#line 912 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.expr) = new Addition( *(yyvsp[-2].expr), *(yyvsp[-1].expr) );
			  Expression::unregister_use( (yyvsp[-2].expr) );
			  Expression::unregister_use( (yyvsp[-1].expr) );
			}
#line 2942 "y.tab.c" /* yacc.c:1646  */
    break;

  case 202:
#line 918 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.expr) = make_subtraction( *(yyvsp[-2].expr), (yyvsp[-1].expr) );
			  Expression::unregister_use( (yyvsp[-2].expr) );
			  Expression::unregister_use( (yyvsp[-1].expr) );
			}
#line 2952 "y.tab.c" /* yacc.c:1646  */
    break;

  case 203:
#line 924 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.expr) = new Multiplication( *(yyvsp[-2].expr), *(yyvsp[-1].expr) );
			  Expression::unregister_use( (yyvsp[-2].expr) );
			  Expression::unregister_use( (yyvsp[-1].expr) );
			}
#line 2962 "y.tab.c" /* yacc.c:1646  */
    break;

  case 204:
#line 930 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.expr) = new Division( *(yyvsp[-2].expr), *(yyvsp[-1].expr) );
			  Expression::unregister_use( (yyvsp[-2].expr) );
			  Expression::unregister_use( (yyvsp[-1].expr) );
			}
#line 2972 "y.tab.c" /* yacc.c:1646  */
    break;

  case 205:
#line 936 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.expr) = (yyvsp[0].appl);
			}
#line 2980 "y.tab.c" /* yacc.c:1646  */
    break;

  case 206:
#line 942 "parser.y" /* yacc.c:1646  */
    {
			  require_fluents();
			  eq_expr = new Value( *(yyvsp[0].num) );
			  delete (yyvsp[0].num);
			}
#line 2990 "y.tab.c" /* yacc.c:1646  */
    break;

  case 207:
#line 948 "parser.y" /* yacc.c:1646  */
    {
			  require_fluents();
			}
#line 2998 "y.tab.c" /* yacc.c:1646  */
    break;

  case 208:
#line 952 "parser.y" /* yacc.c:1646  */
    {
			  eq_expr = new Addition( *(yyvsp[-2].expr), *(yyvsp[-1].expr) );
			  Expression::unregister_use( (yyvsp[-2].expr) );
			  Expression::unregister_use( (yyvsp[-1].expr) );
			}
#line 3008 "y.tab.c" /* yacc.c:1646  */
    break;

  case 209:
#line 958 "parser.y" /* yacc.c:1646  */
    {
			  require_fluents();
			}
#line 3016 "y.tab.c" /* yacc.c:1646  */
    break;

  case 210:
#line 962 "parser.y" /* yacc.c:1646  */
    {
			  eq_expr = make_subtraction( *(yyvsp[-2].expr), (yyvsp[-1].expr) );
			}
#line 3024 "y.tab.c" /* yacc.c:1646  */
    break;

  case 211:
#line 966 "parser.y" /* yacc.c:1646  */
    {
			  require_fluents();
			}
#line 3032 "y.tab.c" /* yacc.c:1646  */
    break;

  case 212:
#line 970 "parser.y" /* yacc.c:1646  */
    {
			  eq_expr = new Multiplication( *(yyvsp[-2].expr), *(yyvsp[-1].expr) );
			  Expression::unregister_use( (yyvsp[-2].expr) );
			  Expression::unregister_use( (yyvsp[-1].expr) );
			}
#line 3042 "y.tab.c" /* yacc.c:1646  */
    break;

  case 213:
#line 976 "parser.y" /* yacc.c:1646  */
    {
			  require_fluents();
			}
#line 3050 "y.tab.c" /* yacc.c:1646  */
    break;

  case 214:
#line 980 "parser.y" /* yacc.c:1646  */
    {
			  eq_expr = new Division( *(yyvsp[-2].expr), *(yyvsp[-1].expr) );
			  Expression::unregister_use( (yyvsp[-2].expr) );
			  Expression::unregister_use( (yyvsp[-1].expr) );
			}
#line 3060 "y.tab.c" /* yacc.c:1646  */
    break;

  case 215:
#line 986 "parser.y" /* yacc.c:1646  */
    {
			  require_fluents();
			  prepare_application((yyvsp[0].str));
			}
#line 3069 "y.tab.c" /* yacc.c:1646  */
    break;

  case 216:
#line 991 "parser.y" /* yacc.c:1646  */
    {
			  eq_expr = make_application();
			}
#line 3077 "y.tab.c" /* yacc.c:1646  */
    break;

  case 217:
#line 995 "parser.y" /* yacc.c:1646  */
    {
			  make_eq_name( (yyvsp[0].str) );
			}
#line 3085 "y.tab.c" /* yacc.c:1646  */
    break;

  case 218:
#line 999 "parser.y" /* yacc.c:1646  */
    {
			  eq_term = make_term( (yyvsp[0].str) );
			  eq_expr = NULL;
			}
#line 3094 "y.tab.c" /* yacc.c:1646  */
    break;

  case 220:
#line 1007 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.expr) = NULL;
			}
#line 3102 "y.tab.c" /* yacc.c:1646  */
    break;

  case 221:
#line 1013 "parser.y" /* yacc.c:1646  */
    {
			  prepare_application( (yyvsp[0].str) );
			}
#line 3110 "y.tab.c" /* yacc.c:1646  */
    break;

  case 222:
#line 1017 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.appl) = make_application();
			}
#line 3118 "y.tab.c" /* yacc.c:1646  */
    break;

  case 223:
#line 1021 "parser.y" /* yacc.c:1646  */
    {
			  prepare_application( (yyvsp[0].str) );
			  (yyval.appl) = make_application();
			}
#line 3127 "y.tab.c" /* yacc.c:1646  */
    break;

  case 224:
#line 1028 "parser.y" /* yacc.c:1646  */
    {
			  prepare_application( (yyvsp[0].str) );
			}
#line 3135 "y.tab.c" /* yacc.c:1646  */
    break;

  case 225:
#line 1032 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.appl) = make_application();
			}
#line 3143 "y.tab.c" /* yacc.c:1646  */
    break;

  case 226:
#line 1036 "parser.y" /* yacc.c:1646  */
    {
			  prepare_application( (yyvsp[0].str) );
			  (yyval.appl) = make_application();
			}
#line 3152 "y.tab.c" /* yacc.c:1646  */
    break;

  case 227:
#line 1043 "parser.y" /* yacc.c:1646  */
    {
			  add_term( (yyvsp[0].str) );
			}
#line 3160 "y.tab.c" /* yacc.c:1646  */
    break;

  case 228:
#line 1047 "parser.y" /* yacc.c:1646  */
    {
			  add_term( (yyvsp[0].str) );
			}
#line 3168 "y.tab.c" /* yacc.c:1646  */
    break;

  case 230:
#line 1054 "parser.y" /* yacc.c:1646  */
    {
			  add_term( (yyvsp[0].str) );
			}
#line 3176 "y.tab.c" /* yacc.c:1646  */
    break;

  case 232:
#line 1061 "parser.y" /* yacc.c:1646  */
    {
			  add_variables( (yyvsp[0].strs), OBJECT_TYPE );
			}
#line 3184 "y.tab.c" /* yacc.c:1646  */
    break;

  case 233:
#line 1065 "parser.y" /* yacc.c:1646  */
    {
			  add_variables( (yyvsp[-1].strs), (yyvsp[0].type) );
			}
#line 3192 "y.tab.c" /* yacc.c:1646  */
    break;

  case 236:
#line 1073 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.strs) = new std::vector<const std::string*>( 1, (yyvsp[0].str) );
			}
#line 3200 "y.tab.c" /* yacc.c:1646  */
    break;

  case 237:
#line 1077 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.strs) = (yyvsp[-1].strs);
			  (yyval.strs)->push_back( (yyvsp[0].str) );
			}
#line 3209 "y.tab.c" /* yacc.c:1646  */
    break;

  case 238:
#line 1084 "parser.y" /* yacc.c:1646  */
    {
			  add_names( (yyvsp[0].strs), OBJECT_TYPE );
			}
#line 3217 "y.tab.c" /* yacc.c:1646  */
    break;

  case 239:
#line 1088 "parser.y" /* yacc.c:1646  */
    {
			  add_names( (yyvsp[-1].strs), (yyvsp[0].type) );
			}
#line 3225 "y.tab.c" /* yacc.c:1646  */
    break;

  case 242:
#line 1096 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.strs) = new std::vector<const std::string*>( 1, (yyvsp[0].str) );
			}
#line 3233 "y.tab.c" /* yacc.c:1646  */
    break;

  case 243:
#line 1100 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.strs) = (yyvsp[-1].strs);
			  (yyval.strs)->push_back( (yyvsp[0].str) );
			}
#line 3242 "y.tab.c" /* yacc.c:1646  */
    break;

  case 244:
#line 1107 "parser.y" /* yacc.c:1646  */
    {
			  require_typing();
			}
#line 3250 "y.tab.c" /* yacc.c:1646  */
    break;

  case 245:
#line 1111 "parser.y" /* yacc.c:1646  */
    {
			 (yyval.type) = (yyvsp[0].type);
		       }
#line 3258 "y.tab.c" /* yacc.c:1646  */
    break;

  case 246:
#line 1117 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.type) = OBJECT_TYPE;
			}
#line 3266 "y.tab.c" /* yacc.c:1646  */
    break;

  case 247:
#line 1121 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.type) = make_type( (yyvsp[0].str) );
			}
#line 3274 "y.tab.c" /* yacc.c:1646  */
    break;

  case 248:
#line 1125 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.type) = make_type( *(yyvsp[-1].types) );
			}
#line 3282 "y.tab.c" /* yacc.c:1646  */
    break;

  case 249:
#line 1131 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.types) = new TypeSet;
			}
#line 3290 "y.tab.c" /* yacc.c:1646  */
    break;

  case 250:
#line 1135 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.types) = new TypeSet;
			  (yyval.types)->insert( make_type( (yyvsp[0].str) ) );
			}
#line 3299 "y.tab.c" /* yacc.c:1646  */
    break;

  case 251:
#line 1140 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.types) = (yyvsp[-1].types);
			}
#line 3307 "y.tab.c" /* yacc.c:1646  */
    break;

  case 252:
#line 1144 "parser.y" /* yacc.c:1646  */
    {
			  (yyval.types) = (yyvsp[-1].types);
			  (yyval.types)->insert( make_type( (yyvsp[0].str) ) );
			}
#line 3316 "y.tab.c" /* yacc.c:1646  */
    break;

  case 254:
#line 1153 "parser.y" /* yacc.c:1646  */
    { delete (yyvsp[0].str); }
#line 3322 "y.tab.c" /* yacc.c:1646  */
    break;

  case 255:
#line 1156 "parser.y" /* yacc.c:1646  */
    { delete (yyvsp[0].str); }
#line 3328 "y.tab.c" /* yacc.c:1646  */
    break;

  case 256:
#line 1159 "parser.y" /* yacc.c:1646  */
    { delete (yyvsp[0].str); }
#line 3334 "y.tab.c" /* yacc.c:1646  */
    break;

  case 257:
#line 1162 "parser.y" /* yacc.c:1646  */
    { delete (yyvsp[0].str); }
#line 3340 "y.tab.c" /* yacc.c:1646  */
    break;

  case 258:
#line 1165 "parser.y" /* yacc.c:1646  */
    { delete (yyvsp[0].str); }
#line 3346 "y.tab.c" /* yacc.c:1646  */
    break;

  case 259:
#line 1168 "parser.y" /* yacc.c:1646  */
    { delete (yyvsp[0].str); }
#line 3352 "y.tab.c" /* yacc.c:1646  */
    break;

  case 260:
#line 1171 "parser.y" /* yacc.c:1646  */
    { delete (yyvsp[0].str); }
#line 3358 "y.tab.c" /* yacc.c:1646  */
    break;

  case 261:
#line 1174 "parser.y" /* yacc.c:1646  */
    { delete (yyvsp[0].str); }
#line 3364 "y.tab.c" /* yacc.c:1646  */
    break;

  case 262:
#line 1177 "parser.y" /* yacc.c:1646  */
    { delete (yyvsp[0].str); }
#line 3370 "y.tab.c" /* yacc.c:1646  */
    break;

  case 263:
#line 1180 "parser.y" /* yacc.c:1646  */
    { delete (yyvsp[0].str); }
#line 3376 "y.tab.c" /* yacc.c:1646  */
    break;

  case 264:
#line 1183 "parser.y" /* yacc.c:1646  */
    { delete (yyvsp[0].str); }
#line 3382 "y.tab.c" /* yacc.c:1646  */
    break;

  case 265:
#line 1186 "parser.y" /* yacc.c:1646  */
    { delete (yyvsp[0].str); }
#line 3388 "y.tab.c" /* yacc.c:1646  */
    break;

  case 266:
#line 1189 "parser.y" /* yacc.c:1646  */
    { delete (yyvsp[0].str); }
#line 3394 "y.tab.c" /* yacc.c:1646  */
    break;

  case 267:
#line 1192 "parser.y" /* yacc.c:1646  */
    { delete (yyvsp[0].str); }
#line 3400 "y.tab.c" /* yacc.c:1646  */
    break;

  case 268:
#line 1195 "parser.y" /* yacc.c:1646  */
    { delete (yyvsp[0].str); }
#line 3406 "y.tab.c" /* yacc.c:1646  */
    break;

  case 269:
#line 1198 "parser.y" /* yacc.c:1646  */
    { delete (yyvsp[0].str); }
#line 3412 "y.tab.c" /* yacc.c:1646  */
    break;

  case 270:
#line 1201 "parser.y" /* yacc.c:1646  */
    { delete (yyvsp[0].str); }
#line 3418 "y.tab.c" /* yacc.c:1646  */
    break;

  case 271:
#line 1204 "parser.y" /* yacc.c:1646  */
    { delete (yyvsp[0].str); }
#line 3424 "y.tab.c" /* yacc.c:1646  */
    break;

  case 272:
#line 1207 "parser.y" /* yacc.c:1646  */
    { delete (yyvsp[0].str); }
#line 3430 "y.tab.c" /* yacc.c:1646  */
    break;

  case 273:
#line 1210 "parser.y" /* yacc.c:1646  */
    { delete (yyvsp[0].str); }
#line 3436 "y.tab.c" /* yacc.c:1646  */
    break;

  case 274:
#line 1213 "parser.y" /* yacc.c:1646  */
    { delete (yyvsp[0].str); }
#line 3442 "y.tab.c" /* yacc.c:1646  */
    break;


#line 3446 "y.tab.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 1260 "parser.y" /* yacc.c:1906  */


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
