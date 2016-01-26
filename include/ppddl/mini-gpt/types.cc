#include "global.h"
#include "types.h"

Type
TypeTable::add_type( const std::string& name )
{
  Type type = last_type() + 1;
  names_.push_back( name );
  types_.insert( std::make_pair( name, type ) );
  if( type > 1 )
    subtype_.push_back( new std::vector<bool>( 2*(type - 1), false ) );
  return( type );
}

Type
TypeTable::add_type( const TypeSet& types )
{
  if( types.empty() )
    return( OBJECT_TYPE );
  else if( types.size() == 1 )
    return( *types.begin() );
  else
    {
      Type type = utypes_.size() + 1;
      utypes_.push_back( &types );
      return( -type );
    }
}

std::pair<Type,bool>
TypeTable::find_type( const std::string& name ) const
{
  std::map<std::string,Type>::const_iterator ti = types_.find( name );
  if( ti != types_.end() )
    return( std::make_pair( (*ti).second, true ) );
  else
    return( std::make_pair( 0, false ) );
}

bool
TypeTable::add_supertype( Type type1, Type type2 )
{
  if( type2 < 0 )
    {
      // add all component types of type2 as supertypes of type1
      const TypeSet& types = *utypes_[-type2 - 1];
      for( TypeSet::const_iterator ti = types.begin(); ti != types.end(); ++ti )
	if( !add_supertype( type1, *ti ) )
	  return( false );
      return( true );
    }
  else if( subtype( type1, type2 ) )
    {
      // the first type is already a subtype of the second type
      return( true );
    }
  else if( subtype( type2, type1 ) )
    {
      // the second type is already a subtype of the first type
      return( false );
    }
  else
    {
      // make all subtypes of type1 subtypes of all supertypes of type2
      Type n = last_type();
      for( Type k = 1; k <= n; ++k )
	if( subtype( k, type1 ) && !subtype( k, type2 ) )
	  {
	    for( Type l = 1; l <= n; ++l )
	      if( subtype( type2, l ) )
		{
		  if( k > l )
		    (*subtype_[k - 2])[2*k - l - 2] = true;
		  else
		    (*subtype_[l - 2])[k - 1] = true;
		}
	  }
      return( true );
    }
}

bool
TypeTable::subtype( Type type1, Type type2 ) const
{
  if( type1 == type2 )
    return( true );
  else if( type1 < 0 )
    {
      // every component type of type1 must be a subtype of type2
      const TypeSet& types = *utypes_[-type1 - 1];
      for( TypeSet::const_iterator ti = types.begin(); ti != types.end(); ++ti )
	if( !subtype( *ti, type2 ) )
	  return( false );
      return( true );
    }
  else if( type2 < 0 )
    {
      // type1 one must be a subtype of some component type of type2
      const TypeSet& types = *utypes_[-type2 - 1];
      for( TypeSet::const_iterator ti = types.begin(); ti != types.end(); ++ti )
	if( subtype( type1, *ti ) )
	  return( true );
      return( false );
    }
  else if( type1 == OBJECT_TYPE )
    return( false );
  else if( type2 == OBJECT_TYPE )
    return( true );
  else if( type1 > type2 )
    return( (*subtype_[type1 - 2])[2*type1 - type2 - 2] );
  else
    return( (*subtype_[type2 - 2])[type1 - 1] );
}

void
TypeTable::components( TypeSet& components, Type type ) const
{
  if( type < 0 )
    components.insert( utypes_[-type - 1]->begin(), utypes_[-type - 1]->end() );
  else if( type != OBJECT_TYPE )
    components.insert( type );
}

void
TypeTable::print_type( std::ostream& os, Type type ) const
{
  if( type < 0 )
    {
      const TypeSet& types = *utypes_[-type - 1];
      os << "(either";
      for( TypeSet::const_iterator ti = types.begin(); ti != types.end(); ++ti )
	os << ' ' << names_[*ti];
      os << ")";
    }
  else if( type == OBJECT_TYPE )
    os << OBJECT_NAME;
  else
    os << names_[type - 1];
}
