#include <limits.h>
#include "global.h"

namespace gpt
{
  bool default_hp = true;
  const char *algorithm = "lrtdp";
  bool domain_analysis = false;
  unsigned dead_end_value = 500; //UINT_MAX;
  unsigned bound = 0;
  unsigned cutoff = UINT_MAX;
  double epsilon = 0;
  bool hash_all = true;
  const char *heuristic = "atom-min-1-backward";
  size_t initial_hash_size = 204800;
  unsigned max_database_size = 32;
  bool noise = false;
  double noise_level = 0;
  unsigned seed = 0;
  int simulations = 0;
  unsigned verbosity = 0;
  unsigned warning_level = 0;
  double heuristic_weight = 1;
  size_t xtra = 0;
  std::stack<heuristic_t*> hstack;
};


#if MEM_DEBUG

void *
operator new( size_t size )
{
  void *result = malloc( size );
  fprintf( stderr, "new %p %d\n", result, size );
  return( result );
}

void *
operator new[]( size_t size )
{
  void *result = malloc( size );
  fprintf( stderr, "new[] %p %d\n", result, size );
  return( result );
}

void
operator delete( void *ptr )
{
  if( ptr )
    {
      fprintf( stderr, "del %p\n", ptr );
      free( ptr );
    }
}

void
operator delete[]( void *ptr )
{
  if( ptr )
    {
      fprintf( stderr, "del[] %p\n", ptr );
      free( ptr );
    }
}

#endif // MEM_DEBUG
