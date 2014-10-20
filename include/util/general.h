#ifndef MDPLIB_GENERAL_H
#define MDPLIB_GENERAL_H

#include <cstring>
#include <unordered_set>
#include <unordered_map>
#include <unistd.h>
#include <chrono>
#include <vector>
#include <thread>

#define dprint1(x) std::cerr << x << std::endl
#define dprint2(x,y) std::cerr << x << " "  << y << std::endl
#define dprint3(x,y,z) std::cerr << x << " "  << y << " " << z << std::endl
#define dprint4(x,y,w,z) std::cerr << x << " "  << y << " " << w << " " << z << std::endl

/**
 * Sleeps the current thread for the given number of milliseconds. This is just a wrapper for
 * function std::this_thread::sleep_for(std::chrono milliseconds).
 */
inline void dsleep(int miliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(miliseconds));
}

/**
 * Computes the next combination (no repetitions) of numbers from 0 to (n-1) chosen k at a time.
 * The method returns true unless no more combinations are possible.
 *
 * This method modifies the given vector in place.
 *
 * For instance, calling nextComb(<0,2>, 3, 2) stores the vector <1,0> and returns true,
 * while calling nextComb(<1,2>, 3, 2) returns false.
 */
inline bool nextComb(std::vector<int>& comb, int n, int k)
{
    int i = k - 1;
    while (i >= 0) {
        if (comb[i] == n - k + i)
            i--;
        else
            break;
    }
    if (i == -1)
        return false;
    comb[i]++;
    for (int j = i + 1; j < k; j++)
        comb[j] = comb[j - 1] + 1;
    return true;
}


/**
 * Computes the next combination (with repetitions) of n elements with k options for each.
 * The method returns true unless no more combinations are possible.
 *
 * This method modifies the given vector in place.
 *
 * For instance, calling nextComb(<1,0>, 2) stores the vector <1,1> and returns true,
 * while calling nextComb(<1,1>, 2) returns false.
 */
inline bool nextCombRep(std::vector<int>& comb, int k)
{
    int j = comb.size() - 1;
    while (j >= 0) {
        comb[j]++;
        if (comb[j] == k) {
            comb[j] = 0;
            j--;
        } else {
            break;
        }
    }
    if (j == -1)
        return false;
    return true;
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
                             double, pair_int_hash,
                             pair_int_equal> PairDoubleMap;

#endif // MDP_LIB_GENERAL_H
