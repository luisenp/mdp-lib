#include <climits>
#include <cmath>

#include "../../include/solvers/HDPSolver.h"

using namespace mlcore;
using namespace std;

namespace mlsolvers
{

int HDPSolver::kappa(double prob, double maxProb)
{
    return int(floor(log2(maxProb) - log2(prob)));
}


void
HDPSolver::computeKappa(list<Successor>& successors, vector<int>& kappaList)
{
    double maxProb = 0.0;
    for (auto const & successor : successors) {
        maxProb = std::max(maxProb, successor.su_prob);
    }
    int i = 0;
    for (auto const & successor : successors) {
        kappaList.at(i) = kappa(successor.su_prob, maxProb);
        i++;
    }
}


bool HDPSolver::dfs(mlcore::State* s, double plaus)
{
    auto curTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(
            curTime - beginTime_).count();
    if (maxTime_ > -1 && duration > maxTime_)
        return false;

    if (plaus > minPlaus_) {
        return false;
    }
    if (s->checkBits(mdplib::SOLVED) ||
            problem_->goal(s) ||
            s->deadEnd()) {
        s->setBits(mdplib::SOLVED);
        solvedStates_.insert(s);
        return false;
    }
    bool neededUpdate = false;
    if (residual(problem_, s) > epsilon_) {
        neededUpdate = true;
        // mini-gpt adds this loop, but this actually worsens convergence.
        // do {
        //     bellmanUpdate(problem_, s);
        // } while (residual(problem_, s) > epsilon_);
        // return true;
    }
    inStack_.insert(s);
    stateStack_.push_back(s);
    indices_[s] = index_;
    low_[s] = index_;
    index_++;

    Action* a = greedyAction(problem_, s);

    if (s->deadEnd()) {
        return false;   // state is a dead-end, nothing to do
    }

    list<Successor> successors = problem_->transition(s, a);
    if (minPlaus_ != INT_MAX)
        computeKappa(successors, kappaList_);
    int i = 0;
    for (auto const & successor : successors) {
        State* next = successor.su_state;
        if (indices_.count(next) == 0) {
            neededUpdate |= dfs(next, plaus + kappaList_.at(i));
            low_[s] = std::min(low_[s], low_[next]);
        } else if (inStack_.count(next) > 0) {
            // State is in the current connected component stack.
            low_[s] = std::min(low_[s], indices_[next]);
        }
        i++;
    }
    if (neededUpdate) {
        bellmanUpdate(problem_, s);
        // same as above (mini-gpt code).
        // do {
        //     bellmanUpdate(problem_, s);
        // } while (residual(problem_, s) > epsilon_);
        //return true;
    } else if (indices_[s] == low_[s]) {
        // State s is the root of a connected component.
        while (true) {
            State* currentState = stateStack_.back();
            stateStack_.pop_back();
            inStack_.erase(currentState);
            currentState->setBits(mdplib::SOLVED);
            solvedStates_.insert(currentState);
            if (currentState == s)
                break;
        }
    }
    return neededUpdate;
}


Action* HDPSolver::solve(State* s0)
{
    int cnt = 0;
    beginTime_ = chrono::high_resolution_clock::now();
    while (!s0->checkBits(mdplib::SOLVED)) {
        auto curTime = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(
            curTime - beginTime_).count();
        if (maxTime_ > -1 && duration >= maxTime_)
            break;
        index_ = 0;
        dfs(s0, 0);
        inStack_.clear();
        indices_.clear();
        low_.clear();
        stateStack_.clear();
        if (cnt++ >= maxTrials_) {
            break;
        }
    }
    return s0->bestAction();
}

} // namespace mlsolvers
