#ifndef MDPLIB_GENERAL_H
#define MDPLIB_GENERAL_H

#include <cstring>
#include <ctime>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <unistd.h>
#include <chrono>
#include <vector>
#include <thread>

extern bool mdplib_debug;

extern std::chrono::time_point<std::chrono::high_resolution_clock> mdplib_tic_;

extern std::chrono::time_point<std::chrono::high_resolution_clock> mdplib_toc_;

// Implementation of c++14 std::make_unique as explained in
// https://herbsutter.com/gotw/_102/
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
void dprint(T t) {
    if (mdplib_debug)
        std::cerr << t << std::endl ;
}

/**
 * Prints all arguments passed separated by spaces.
 */
template<typename T, typename... Args>
void dprint(T t, Args... args) /* recursive variadic function*/ {
    if (mdplib_debug) {
        std::cerr << t << " ";
        dprint(args...);
    }
}

std::string debug_pad(int n);

/** Stores the current time in mdplib_tic. */
void mdplib_tic();

/** Stores the current time in mdplib_toc. */
void mdplib_toc();

/** Returns the time elapsed in nanoseconds between tic and toc. */
long long mdplib_elapsed_nano();

/** Returns the time elapsed in microseconds between tic and toc. */
long long mdplib_elapsed_micro();

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


/**
 * Computes the time left on a clock, according to the current time,
 * the given starting time and the given max time.
 * If the time left is <= 0, the method returns true,
 * otherwise it returns false.
 */
bool timeHasRunOut(
    time_t startingTime, time_t maxTime, time_t* timeLeft = nullptr);

struct pair_int_equal {
  bool operator() (std::pair<int,int> p1, std::pair<int,int> p2) const {
    return p1.first == p2.first && p1.second == p2.second;
  }
};

struct pair_int_hash {
    std::size_t operator()(const std::pair<int,int> & v) const {
        return v.first * 31 + v.second;
    }
};

typedef std::unordered_set< std::pair <int,int> ,
                            pair_int_hash,
                            pair_int_equal> IntPairSet;

typedef std::unordered_map< std::pair <int,int> ,
                             double, pair_int_hash,
                             pair_int_equal> PairDoubleMap;

#endif // MDP_LIB_GENERAL_H
