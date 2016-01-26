#include "domains.h"
#include "formulas.h"
#include "problems.h"

Domain::DomainMap Domain::domains = Domain::DomainMap();

Domain::DomainMap::const_iterator Domain::begin( void )
{
  return( domains.begin() );
}

Domain::DomainMap::const_iterator Domain::end( void )
{
  return( domains.end() );
}

const Domain* 
Domain::find( const std::string& name )
{
  DomainMap::const_iterator di = domains.find( name );
  return( di != domains.end() ? (*di).second : NULL );
}

void
Domain::clear( void )
{
  DomainMap::const_iterator di = begin();
  while( di != end() )
    {
      delete (*di).second;
      di = begin();
    }
  domains.clear();
}

Domain::Domain( const std::string& name )
  : name_(name)
{
  const Domain* d = find( name );
  if( d != NULL )
    delete d;
  domains[name] = this;
}

Domain::~Domain()
{
  domains.erase( name() );
  for( ActionSchemaMap::const_iterator ai = actions_.begin(); ai != actions_.end(); ++ai )
    delete (*ai).second;
}

void
Domain::add_action( const ActionSchema& action )
{
  actions_.insert( std::make_pair( action.name(), &action ) );
}

const ActionSchema*
Domain::find_action( const std::string& name ) const
{
  ActionSchemaMap::const_iterator ai = actions_.find( name );
  return( ai != actions_.end() ? (*ai).second : NULL );
}

void
Domain::compatible_constants( ObjectList& constants, Type type ) const
{
  Object last = terms().last_object();
  for( Object i = terms().first_object(); i <= last; ++i )
    if( types().subtype(terms().type( i ), type ) )
      constants.push_back( i );
}

void
Domain::instantiated_actions( ActionList& actions,
			      std::map<const StateFormula*,const Atom*> &hash,
			      const problem_t& problem ) const
{
  // instantiate actions
  for( ActionSchemaMap::const_iterator ai = actions_.begin(); ai != actions_.end(); ++ai )
    (*ai).second->instantiations( actions, problem );

  // instantiate internal hash
  VariableList context, fvars;
  std::map<const StateFormula*,const Atom*>::const_iterator hi;
  for( hi = internal_hash_.begin(); hi != internal_hash_.end(); ++hi )
    {
      context.clear();
      fvars.clear();
      (*hi).first->free_vars( context, fvars );
      if( fvars.size() == 0 )
	{
	  SubstitutionMap subst;
	  const StateFormula *f = &(*hi).first->instantiation( subst, problem );
	  const StateFormula *a = &(*hi).second->instantiation( subst, problem );
	  hash[f] = (const Atom*)a;
	  StateFormula::register_use( (*hi).second );
	}
      else
	{
	  SubstitutionMap args;
	  std::vector<ObjectList> arguments( fvars.size(), ObjectList() );
	  std::vector<ObjectList::const_iterator> next_arg;
	  for( size_t i = 0; i < fvars.size(); ++i )
	    {
	      problem.compatible_objects( arguments[i],
					  problem.domain().terms().type( fvars[i] ) );
	      if( arguments[i].empty() ) return;
	      next_arg.push_back( arguments[i].begin() );
	    }

	  std::stack<std::pair<const StateFormula*,const StateFormula*> > partial;
	  partial.push( std::make_pair( (*hi).first, (*hi).second ) );
	  StateFormula::register_use( partial.top().first );
	  StateFormula::register_use( partial.top().second );
	  for( size_t i = 0; i < fvars.size(); )
	    {
	      SubstitutionMap pargs;
	      args.insert( std::make_pair( fvars[i], *next_arg[i] ) );
	      pargs.insert( std::make_pair( fvars[i], *next_arg[i] ) );
	      const StateFormula *f = &partial.top().first->instantiation( pargs, problem );
	      const StateFormula *a = &partial.top().second->instantiation( pargs, problem );
	      partial.push( std::make_pair( f, a ) );

	      if( i + 1 == fvars.size() )
		{
		  StateFormula::register_use( f );
		  StateFormula::register_use( a );
		  hash[f] = (const Atom*)a;

		  for( int j = i; j >= 0; --j )
		    {
		      StateFormula::unregister_use( partial.top().first );
		      StateFormula::unregister_use( partial.top().second );
		      partial.pop();
		      args.erase( fvars[j] );
		      ++next_arg[j];
		      if( next_arg[j] == arguments[j].end() )
			{
			  if( j == 0 )
			    {
			      i = fvars.size();
			      break;
			    }
			  else
			    next_arg[j] = arguments[j].begin();
			}
		      else
			{
			  i = j;
			  break;
			}
		    }
		}
	      else
		++i;
	    }
	}
    }
}

void
Domain::analyze_and_simplify( void )
{
  VariableList context, fvars;
  std::map<const StateFormula*,const Atom*> hash;
  std::map<const StateFormula*,const Atom*>::const_iterator hi, it;

  for( ActionSchemaMap::const_iterator ai = actions_.begin(); ai != actions_.end(); ++ai )
    (*ai).second->analyze( predicates(), terms(), hash );

  if( gpt::verbosity >= 350 )
    {
      std::cout << "result of Domain Analysis:" << std::endl;
      for( hi = hash.begin(); hi != hash.end(); ++hi )
	{
	  context.clear();
	  fvars.clear();
	  (*hi).first->free_vars( context, fvars );
      
	  std::cout << "  ";
	  (*hi).second->print( std::cout, predicates(), functions(), terms() );
	  std::cout << ":";
	  (*hi).first->print( std::cout, predicates(), functions(), terms() );
	  std::cout << std::endl;
	}
      std::cout << "**" << std::endl;
    }

  // rewrite action schemas
  for( ActionSchemaMap::const_iterator ai = actions_.begin(); ai != actions_.end(); ++ai )
    {
      const ActionSchema *action = &(*ai).second->rewrite( hash );
      delete (*ai).second;
      actions_[(*ai).first] = action;
    }

  // setup internal-hash and clear hash
  for( hi = hash.begin(); hi != hash.end(); ++hi )
    {
      for( it = internal_hash_.begin(); it != internal_hash_.end(); ++it )
	if( (*hi).second->predicate() == (*it).second->predicate() )
	  break;

      if( it == internal_hash_.end() )
	{
	  StateFormula::register_use( (*hi).first );
	  StateFormula::register_use( (*hi).second );
	  internal_hash_.insert( *hi );
	}
      StateFormula::unregister_use( (*hi).first );
      StateFormula::unregister_use( (*hi).second );
    }
}

std::ostream&
operator<<( std::ostream& os, const Domain& d )
{
  os << "name: " << d.name();
  os << std::endl << "types:";
  for( Type i = d.types().first_type(); i <= d.types().last_type(); ++i )
    {
      os << std::endl << "  ";
      d.types().print_type( os, i );
      bool first = true;
      for( Type j = d.types().first_type(); j <= d.types().last_type(); ++j )
	{
	  if( (i != j) && d.types().subtype( i, j ) )
	    {
	      if( first )
		{
		  os << " -";
		  first = false;
		}
	      os << ' ';
	      d.types().print_type( os, j );
	    }
	}
    }
  os << std::endl << "constants:";
  for( Object i = d.terms().first_object(); i <= d.terms().last_object(); ++i )
    {
      os << std::endl << "  ";
      d.terms().print_term( os, i );
      os << " - ";
      d.types().print_type( os, d.terms().type( i ) );
    }
  os << std::endl << "predicates:";
  for( Predicate i = d.predicates().first_predicate(); i <= d.predicates().last_predicate(); ++i )
    {
      os << std::endl << "  (";
      d.predicates().print_predicate( os, i );
      size_t arity = d.predicates().arity( i );
      for( size_t j = 0; j < arity; ++j )
	{
	  os << " ?v - ";
	  d.types().print_type( os, d.predicates().parameter( i, j ) );
	}
      os << ")";
      if( d.predicates().static_predicate( i ) )
	os << " <static>";
    }
  os << std::endl << "functions:";
  for( Function i = d.functions().first_function(); i <= d.functions().last_function(); ++i )
    {
      os << std::endl << "  (";
      d.functions().print_function( os, i );
      size_t arity = d.functions().arity( i );
      for( size_t j = 0; j < arity; ++j )
	{
	  os << " ?v - ";
	  d.types().print_type( os, d.functions().parameter( i, j ) );
	}
      os << ") - " << NUMBER_NAME;
      if( d.functions().static_function( i ) )
	os << " <static>";
    }
  os << std::endl << "actions:";
  for( ActionSchemaMap::const_iterator ai = d.actions_.begin(); ai != d.actions_.end(); ++ai )
    {
      os << std::endl;
      (*ai).second->print( os, d.predicates(), d.functions(), d.terms() );
    }
  return( os );
}
