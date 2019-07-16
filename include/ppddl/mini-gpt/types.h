#ifndef TYPES_H
#define TYPES_H

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <unordered_set>

typedef int Type;
const Type OBJECT_TYPE = 0;

class TypeList : public std::vector<Type> { };
class TypeSet : public std::unordered_set<Type> { };

class TypeTable
{
  std::vector<std::string> names_;
  std::map<std::string,Type> types_;
  std::vector<std::vector<bool>*> subtype_;
  std::vector<const TypeSet*> utypes_;

public:
  Type add_type( const std::string& name );
  Type add_type( const TypeSet& types );
  std::pair<Type,bool> find_type( const std::string& name ) const;
  Type first_type( void ) const { return( OBJECT_TYPE ); }
  Type last_type( void ) const { return( names_.size() ); }
  bool add_supertype( Type type1, Type type2 );
  bool subtype( Type type1, Type type2 ) const;
  void components( TypeSet& components, Type type ) const;
  void print_type( std::ostream& os, Type type ) const;
};

#endif // TYPES_H
