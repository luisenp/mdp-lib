#include <cstring>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <unistd.h>
#include <chrono>
#include <vector>
#include <thread>

#include "../../include/util/general.h"

bool mdplib_debug = false;
std::chrono::time_point<std::chrono::high_resolution_clock> mdplib_tic_ =
    std::chrono::high_resolution_clock::now();
std::chrono::time_point<std::chrono::high_resolution_clock> mdplib_toc_ =
    std::chrono::high_resolution_clock::now();

std::string debug_pad(int n) {
    char buf[512];
    sprintf(buf, "%*s", n, "");
    return std::string(buf);
}

void dsleep(int miliseconds) {
    if (mdplib_debug)
        std::this_thread::sleep_for(std::chrono::milliseconds(miliseconds));
}

void mdplib_tic() {
    mdplib_tic_ = std::chrono::high_resolution_clock::now();
}

void mdplib_toc() {
    mdplib_toc_ = std::chrono::high_resolution_clock::now();
}

long long mdplib_elapsed_nano() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        mdplib_toc_ - mdplib_tic_).count();
}

long long mdplib_elapsed_micro() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
        mdplib_toc_ - mdplib_tic_).count();
}

bool nextComb(std::vector<int>& comb, int n, int k) {
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


bool nextCombRep(std::vector<int>& comb, int k) {
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


bool timeHasRunOut(time_t startingTime, time_t maxTime, time_t* timeLeft) {
    time_t elapsedTime = time(nullptr) - startingTime;
    if (timeLeft != nullptr)
        *timeLeft = maxTime - elapsedTime;
    return elapsedTime > maxTime;
}
