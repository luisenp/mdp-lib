#include "predicates.h"

PredicateTable::~PredicateTable()
{
  for( std::vector<TypeList*>::const_iterator pi = parameters_.begin(); pi != parameters_.end(); ++pi )
    delete *pi;
}

Predicate 
PredicateTable::add_predicate( const std::string& name )
{
  Predicate predicate = last_predicate() + 1;
  names_.push_back( name );
  predicates_.insert( std::make_pair( name, predicate ) );
  parameters_.push_back( new TypeList() );
  static_predicates_.insert( predicate );
  return( predicate );
}

std::pair<Predicate,bool>
PredicateTable::find_predicate( const std::string& name ) const
{
  std::map<std::string,Predicate>::const_iterator pi = predicates_.find( name );
  if( pi != predicates_.end() )
    return( std::make_pair( (*pi).second, true ) );
  else
    return( std::make_pair( 0, false ) );
}

void 
PredicateTable::print_predicate( std::ostream& os, Predicate predicate) const
{
  os << names_[predicate];
}
