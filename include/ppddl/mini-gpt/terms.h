#ifndef TERMS_H
#define TERMS_H

#include "global.h"
#include "types.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <unordered_map>

typedef int Term;
typedef size_t Object;
typedef int Variable;

const Term NULL_TERM = 0;

inline bool is_object( Term term ) { return( term > 0 ); }
inline bool is_variable( Term term ) { return( term < 0 ); }

class SubstitutionMap : public std::unordered_map<Variable,Object> { };
class VarSubstMap : public std::unordered_map<Variable,Variable> { };
class TermList : public std::vector<Term> { };
class ObjectList : public std::vector<Object> { };
class VariableList : public std::vector<Variable> { };

class TermTable
{
  const TermTable *parent_;
  std::vector<std::string> names_;
  std::map<std::string,Object> objects_;
  TypeList object_types_;
  TypeList variable_types_;

public:
  TermTable() : parent_(NULL) { }
  TermTable( const TermTable& parent ) : parent_(&parent) { }
  Object add_object( const std::string& name, Type type );
  Variable add_variable( Type type );
  std::pair<Object,bool> find_object( const std::string& name ) const;
  Object first_object( void ) const
    {
      return( parent_ != NULL ? parent_->last_object() + 1 : 1 );
    }
  Object last_object( void ) const
    {
      return( first_object() + names_.size() - 1 );
    }
  Variable first_variable( void ) const
    {
      return( parent_ != NULL ? parent_->last_variable() - 1 : -1 );
    }
  Variable last_variable( void ) const
    {
      return( first_variable() - variable_types_.size() + 1 );
    }
  void set_type( Term term, Type type );
  Type type( Term term ) const;
  void print_term( std::ostream& os, Term term ) const;
  TermTable& operator=( const TermTable &table );
};

#endif // TERMS_H
