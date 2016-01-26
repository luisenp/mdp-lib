#include "terms.h"

Object
TermTable::add_object( const std::string& name, Type type )
{
  Object object = last_object() + 1;
  names_.push_back( name );
  objects_.insert( std::make_pair( name, object ) );
  object_types_.push_back( type );
  return( object );
}

Variable
TermTable::add_variable( Type type )
{
  Variable variable = last_variable() - 1;
  variable_types_.push_back( type );
  return( variable );
}

std::pair<Object,bool>
TermTable::find_object( const std::string& name ) const
{
  std::map<std::string,Object>::const_iterator oi = objects_.find( name );
  if( oi != objects_.end() )
    return( std::make_pair( (*oi).second, true ) );
  else if( parent_ != NULL )
    return( parent_->find_object( name ) );
  else
    return( std::make_pair( 0, false ) );
}

void
TermTable::set_type( Term term, Type type )
{
  if( is_object( term ) )
    object_types_[term - first_object()] = type;
  else
    variable_types_[first_variable() - term] = type;
}

Type
TermTable::type( Term term ) const
{
  if( is_object( term ) )
    {
      if( Object( term ) < first_object() )
	return( parent_->type( term ) );
      else
	return( object_types_[term - first_object()] );
      
    }
  else
    {
      if( term > first_variable() )
	return( parent_->type( term ) );
      else
	return( variable_types_[first_variable() - term] );
    }
}

void
TermTable::print_term( std::ostream& os, Term term ) const
{
  if( is_object( term ) )
    {
      if( Object( term ) < first_object() )
	parent_->print_term( os, term );
      else
	os << names_[term - first_object()];
    }
  else
    os << "?v" << -term;
}

TermTable&
TermTable::operator=( const TermTable &table )
{
  parent_ = table.parent_;
  names_ = table.names_;
  objects_ = table.objects_;
  object_types_ = table.object_types_;
  variable_types_ = table.variable_types_;
  return( *this );
}
