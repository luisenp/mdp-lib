#ifndef MDPLIB_GENERAL_H
#define MDPLIB_GENERAL_H

#include <cstring>
#include <unordered_set>
#include <unordered_map>

#include "rational.h"

namespace mdplib
{
    const int dead_end_cost = 10000;
}


struct pair_int_equal {
  bool operator() (std::pair<int,int> p1, std::pair<int,int> p2) const {
    return p1.first == p2.first && p1.second == p2.second;
  }
};

struct pair_int_hash {
    inline std::size_t operator()(const std::pair<int,int> & v) const {
        return v.first*31+v.second;
    }
};

typedef std::unordered_set< std::pair <int,int> , pair_int_hash, pair_int_equal> IntPairSet;

typedef std::unordered_map< std::pair <int,int> ,
                             Rational, pair_int_hash,
                             pair_int_equal> PairRationalMap;

#endif // MDP_LIB_GENERAL_H
