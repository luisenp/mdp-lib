#include "functions.h"

FunctionTable::~FunctionTable()
{
  for( std::vector<TypeList*>::const_iterator pi = parameters_.begin(); pi != parameters_.end(); ++pi )
    delete *pi;
}

Function
FunctionTable::add_function( const std::string& name )
{
  Function function = last_function() + 1;
  names_.push_back( name );
  functions_.insert( std::make_pair( name, function ) );
  parameters_.push_back( new TypeList() );
  static_functions_.insert( function );
  return( function );
}

std::pair<Function,bool>
FunctionTable::find_function( const std::string& name ) const
{
  std::map<std::string,Function>::const_iterator fi = functions_.find( name );
  if( fi != functions_.end() )
    return( std::make_pair( (*fi).second, true ) );
  else
    return( std::make_pair( 0,false ) );
}

void
FunctionTable::print_function( std::ostream& os, Function function ) const
{
  os << names_[function];
}
