#ifndef HASHING_H
#define HASHING_H

#if !defined __GNUC__ || __GNUC__ < 3
# include <hash_map>
# include <hash_set>
namespace hashing
{
  using ::hash_map;
  using ::hash_multimap;
  using ::hash_set;
  using ::hash;
};
#else
# include <ext/hash_map>
# include <ext/hash_set>
# if __GNUC_MINOR__ == 0
namespace hashing = std;
# else
namespace hashing = ::__gnu_cxx;
# endif
#endif

#if defined __GNUC__ && __GNUC__ >= 3
# if __GNUC_MINOR__ == 0
namespace std
# else
namespace __gnu_cxx
# endif
{
#endif
  template<typename T> class hash<const T*>
  {
  public:
    size_t operator()( const T* p ) const { return( size_t(p) ); }
  };
#if defined __GNUC__ && __GNUC__ >= 3
};
#endif

#endif // HASHING_H
