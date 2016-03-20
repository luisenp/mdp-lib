#include <climits>

#include "../../include/solvers/HDPSolver.h"

using namespace mlcore;

namespace mlsolvers
{

bool HDPSolver::dfs(mlcore::State* s)
{
    if (s->checkBits(mdplib::SOLVED) ||
            problem_->goal(s) ||
            s->deadEnd()) {
        s->setBits(mdplib::SOLVED);
        return false;
    }
    if (residual(problem_, s) > epsilon_) {
        bellmanUpdate(problem_, s);
        return true;
    }
    inStack_.insert(s);
    stateStack_.push_back(s);
    indices_[s] = index_;
    low_[s] = index_;
    index_++;

    bool neededUpdate = false;
    Action* a = greedyAction(problem_, s);
    for (auto const & successor : problem_->transition(s, a)) {
        State* next = successor.su_state;
        if (indices_.count(next) == 0) {
            neededUpdate |= dfs(next);
            low_[s] = std::min(low_[s], low_[next]);
        } else if (inStack_.count(next) > 0) {
            // State is in the current connected component stack.
            low_[s] = std::min(low_[s], indices_[next]);
        }
    }
    if (neededUpdate) {
        bellmanUpdate(problem_, s);
        return true;
    } else if (indices_[s] == low_[s]) {
        // State s is the root of a connected component.
        while (true) {
            State* currentState = stateStack_.back();
            stateStack_.pop_back();
            inStack_.erase(currentState);
            currentState->setBits(mdplib::SOLVED);
            if (currentState == s)
                break;
        }
    }
    return neededUpdate;
}

Action* HDPSolver::solve(State* s0)
{
    while (!s0->checkBits(mdplib::SOLVED)) {
        index_ = 0;
        dfs(s0);
        inStack_.clear();
        indices_.clear();
        low_.clear();
        stateStack_.clear();
    }
}

} // namespace mlsolvers
