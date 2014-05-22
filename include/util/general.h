#ifndef MDPLIB_GENERAL_H
#define MDPLIB_GENERAL_H

#include <cstring>
#include <unordered_set>
#include <unordered_map>

struct pair_int_hash {
    inline std::size_t operator()(const std::pair<int,int> & v) const {
        return v.first*31+v.second;
    }
};

typedef std::unordered_set< std::pair <int,int> , pair_int_hash> IntPairSet;

typedef std::unordered_map< std::pair <int,int> , int, pair_int_hash> IntPairMap;

#endif // MDP_LIB_GENERAL_H
