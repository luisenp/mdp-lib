#ifndef PREDICATES_H
#define PREDICATES_H

#include "types.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <unordered_set>

typedef int Predicate;
class PredicateSet : public std::unordered_set<Predicate> { };

class PredicateTable
{
  std::vector<std::string> names_;
  std::map<std::string,Predicate> predicates_;
  std::vector<TypeList*> parameters_;
  PredicateSet static_predicates_;

public:
  ~PredicateTable();
  Predicate add_predicate( const std::string& name );
  std::pair<Predicate,bool> find_predicate( const std::string& name ) const;
  Predicate first_predicate( void ) const { return( 0 ); }
  Predicate last_predicate( void ) const { return( names_.size() - 1 ); }
  void add_parameter( Predicate predicate, Type type )
    {
      parameters_[predicate]->push_back( type );
    }
  const std::string& name( Predicate predicate ) const { return( names_[predicate] ); }
  size_t arity( Predicate predicate ) const { return( parameters_[predicate]->size() ); }
  Type parameter( Predicate predicate, size_t i ) const
    {
      return( (*parameters_[predicate])[i] );
    }
  void make_dynamic( Predicate predicate ) { static_predicates_.erase( predicate ); }
  bool static_predicate( Predicate predicate ) const
    {
      return( static_predicates_.find( predicate ) != static_predicates_.end() );
    }
  void print_predicate( std::ostream& os, Predicate predicate ) const;
};

#endif // PREDICATES_H
