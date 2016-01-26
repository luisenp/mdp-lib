#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>
#include <string>
#include <memory>
#include <stack>

#ifndef UCHAR_MAX
#define UCHAR_MAX       255
#endif
#ifndef USHORT_MAX
#define USHORT_MAX      65535
#endif

#define NUMBER_NAME     "number"
#define OBJECT_NAME     "object"

#define MAX(x,y)        ((x)>(y)?(x):(y))
#define MIN(x,y)        ((x)<(y)?(x):(y))

#define DISP_SIZE       128
#define DISP_INT_SIZE   128

class heuristic_t;

namespace gpt
{
  extern bool default_hp;
  extern const char *algorithm;
  extern bool domain_analysis;
  extern unsigned dead_end_value;
  extern unsigned cutoff;
  extern double epsilon;
  extern bool hash_all;
  extern const char *heuristic;
  extern size_t initial_hash_size;
  extern unsigned max_database_size;
  extern bool noise;
  extern double noise_level;
  extern unsigned seed;
  extern int simulations;
  extern unsigned verbosity;
  extern unsigned warning_level;
  extern double heuristic_weight;
  extern size_t xtra;
  extern std::stack<heuristic_t*> hstack;
};

typedef unsigned char uchar_t;
typedef unsigned short ushort_t;

inline void
notify( void *ptr, const char *name )
{
#ifdef MEM_DEBUG
  fprintf( stderr, "notify %s %p\n", name, ptr );
#endif
}

#endif // GLOBAL_H
