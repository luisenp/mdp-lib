#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "types.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <unordered_set>

typedef int Function;

class FunctionSet : public std::unordered_set<Function> { };

class FunctionTable
{
  std::vector<std::string> names_;
  std::map<std::string, Function> functions_;
  std::vector<TypeList*> parameters_;
  FunctionSet static_functions_;

public:
  ~FunctionTable();
  Function add_function( const std::string& name );
  std::pair<Function,bool> find_function( const std::string& name ) const;
  Function first_function( void ) const { return( 0 ); }
  Function last_function( void ) const { return( names_.size() - 1 ); }
  void add_parameter( Function function, Type type) 
    {
      parameters_[function]->push_back( type );
    }
  const std::string& name( Function function ) const { return( names_[function] ); }
  size_t arity( Function function ) const { return( parameters_[function]->size() ); }
  Type parameter( Function function, size_t i ) const
    {
      return( (*parameters_[function])[i] );
    }
  void make_dynamic( Function function ) { static_functions_.erase( function ); }
  bool static_function( Function function ) const
    {
      return( static_functions_.find( function ) != static_functions_.end() );
    }
  void print_function( std::ostream& os, Function function ) const;
};

#endif // FUNCTIONS_H
