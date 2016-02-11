#ifndef MDPLIB_GENERAL_H
#define MDPLIB_GENERAL_H

#include <cstring>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <unistd.h>
#include <chrono>
#include <vector>
#include <thread>

extern bool mdplib_debug;

// TODO: Implement this using variadic function/templates.
// Current implementation quite ugly

template <class T>
void dprint1(T x)
{
    if (mdplib_debug)
        std::cerr << x << std::endl;
}

template <class T1, class T2>
void dprint2(T1 x, T2 y)
{
    if (mdplib_debug)
        std::cerr << x << " " << y << std::endl;
}

template <class T1, class T2, class T3>
void dprint3(T1 x, T2 y, T3 z)
{
    if (mdplib_debug)
        std::cerr << x << " " << y << " " << z << std::endl;
}

template <class T1, class T2, class T3, class T4>
void dprint4(T1 x, T2 y, T3 z, T4 w)
{
    if (mdplib_debug)
        std::cerr << x << " " << y << " " << z << " " << w << std::endl;
}

template <class T1, class T2, class T3, class T4, class T5>
void dprint5(T1 x, T2 y, T3 z, T4 w, T5 xx)
{
    if (mdplib_debug)
        std::cerr << x << " " << y << " " << z << " " <<
            w << " " << xx << std::endl;
}

/**
 * Sleeps the current thread for the given number of milliseconds.
 * This is just a wrapper for
 * function std::this_thread::sleep_for(std::chrono milliseconds).
 */
void dsleep(int miliseconds);

/**
 * Computes the next combination (no repetitions) of numbers from 0 to (n-1)
 * chosen k at a time.
 * The method returns true unless no more combinations are possible.
 *
 * This method modifies the given vector in place.
 *
 * For instance, calling nextComb(<0,2>, 3, 2) stores the vector <1,0>
 * and returns true, while calling nextComb(<1,2>, 3, 2) returns false.
 */
bool nextComb(std::vector<int>& comb, int n, int k);

/**
 * Computes the next combination (with repetitions) of n elements with k options
 * for each.
 * The method returns true unless no more combinations are possible.
 *
 * This method modifies the given vector in place.
 *
 * For instance, calling nextComb(<1,0>, 2) stores the vector <1,1> and
 * returns true, while calling nextComb(<1,1>, 2) returns false.
 */
bool nextCombRep(std::vector<int>& comb, int k);

struct pair_int_equal {
  bool operator() (std::pair<int,int> p1, std::pair<int,int> p2) const {
    return p1.first == p2.first && p1.second == p2.second;
  }
};

struct pair_int_hash {
    std::size_t operator()(const std::pair<int,int> & v) const {
        return v.first*31+v.second;
    }
};

typedef std::unordered_set< std::pair <int,int> ,
                            pair_int_hash,
                            pair_int_equal> IntPairSet;

typedef std::unordered_map< std::pair <int,int> ,
                             double, pair_int_hash,
                             pair_int_equal> PairDoubleMap;

#endif // MDP_LIB_GENERAL_H
