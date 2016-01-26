#ifndef DOMAINS_H
#define DOMAINS_H

#include "actions.h"
#include "functions.h"
#include "predicates.h"
#include "terms.h"
#include "types.h"
#include "requirements.h"
#include <iostream>
#include <map>
#include <string>

class Problem;

class Domain
{
public:
  Requirements requirements;
  class DomainMap : public std::map<std::string,const Domain*> { };
  static DomainMap::const_iterator begin( void );
  static DomainMap::const_iterator end( void );
  static const Domain* find( const std::string& name );
  static void clear( void );

private:
  static DomainMap domains;
  std::string name_;
  TypeTable types_;
  PredicateTable predicates_;
  FunctionTable functions_;
  TermTable terms_;
  ActionSchemaMap actions_;
  std::map<const StateFormula*,const Atom*> internal_hash_;

  friend std::ostream& operator<<( std::ostream& os, const Domain& d );

public:
  Domain( const std::string& name );
  ~Domain();

  const std::string& name( void ) const { return( name_ ); }
  TypeTable& types( void ) { return( types_ ); }
  const TypeTable& types( void ) const { return( types_ ); }
  PredicateTable& predicates( void ) { return( predicates_ ); }
  const PredicateTable& predicates( void ) const { return( predicates_ ); }
  FunctionTable& functions( void ) { return( functions_ ); }
  const FunctionTable& functions( void ) const { return( functions_ ); }
  TermTable& terms( void ) { return( terms_ ); }
  const TermTable& terms( void ) const { return( terms_ ); }
  void add_action( const ActionSchema& action );
  const ActionSchema* find_action( const std::string& name ) const;
  void compatible_constants( ObjectList& constants, Type type ) const;
  void instantiated_actions( ActionList& actions, 
			     std::map<const StateFormula*,const Atom*> &hash,
			     const problem_t& problem ) const;
  void analyze_and_simplify( void );
};

std::ostream& operator<<( std::ostream& os, const Domain& d );

#endif // DOMAINS_H
