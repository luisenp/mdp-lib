#include "../../include/util/general.h"
#include "../../include/solvers/solver.h"
#include "../../include/solvers/LexiLAOStarSolver.h"

#include <ctime>

namespace mlsolvers
{

void LexiLAOStarSolver::solveLevel(mlcore::State* s, int level, mllexi::LexiState*& unsolved)
{
    mllexi::LexiState* s0 = (mllexi::LexiState *) s;
    clock_t startTime = clock();
    int totalExpanded = 0;
    int countExpanded = 0;
    double error = mdplib::dead_end_cost;
    while (true) {
        do {
            visited_.clear();
            unsolved = nullptr;
            countExpanded = expand(s0, level, unsolved);
            if (level > 0 && unsolved != nullptr) {
                return;
            }
            totalExpanded += countExpanded;

        } while (countExpanded != 0);

        while (true) {
            visited_.clear();
            error = testConvergence(s0, level);
            if (error < epsilon_) {
                addSolved(s0);
                unsolved = nullptr;
                return;
            }
            if (error > mdplib::dead_end_cost) {
                break;  // BPSG changed, must expand tip nodes again
            }
        }
    }
}

mlcore::Action* LexiLAOStarSolver::solve(mlcore::State* s)
{
    mllexi::LexiState* unsolved = nullptr;
//    solveLevel(s, problem_->size() - 1, unsolved);

    double bestValue, worstValue, bestValue2;
    double wBest = 1.0, wWorst = 0.0;

    mllexi::LexiState* state = (mllexi::LexiState* ) s;

    weights_[0] = 1.0; weights_[1] = 0.0;
    solveLevel(s, 0, unsolved);
                dprint4("BEST ", state->lexiCost()[0], " ", state->lexiCost()[1]);
    bestValue = state->lexiCost()[0];
    bestValue2 = state->lexiCost()[1];
    double thrValue = (1.0 + problem_->slack()) * s->cost();
                dprint2("THRESHOLD ", thrValue);
    weights_[0] = 0.0; weights_[1] = 1.0;
    initialized_.clear();
    for (mlcore::State* x : problem_->states()) {
        ((mllexi::LexiState * ) x)->resetCost(weights_, -1);
        ((mllexi::LexiState * ) x)->setBestAction(nullptr);
    }
    solveLevel(s, 0, unsolved);
    double cur = state->lexiCost()[0];
    worstValue = state->lexiCost()[0];
                dprint4("WORST ", state->lexiCost()[0], " ", state->lexiCost()[1]);

    if (cur <= thrValue)
        return s->bestAction();

    thresholdCost0_ = thrValue;
    double lo = 0.0, hi = 1.0;
    int keepIdx = 1;
    for (int i = 0; i < 10; i++) {
//        double slope = (worstValue - bestValue) / (wWorst - wBest);
//        double mid = wBest + (thrValue - bestValue) / slope;
        double mid = (wBest + wWorst) / 2;
        initialized_.clear();
        weights_[0] = mid; weights_[1] = (1.0 - mid);
        for (mlcore::State* x : problem_->states()) {
            ((mllexi::LexiState * ) x)->resetCost(weights_, keepIdx);
            ((mllexi::LexiState * ) x)->setBestAction(nullptr);
        }
        solveLevel(s, 0, unsolved);

        cur = state->lexiCost()[0];;
        dprint4("WEIGHT ", mid, " RATIO ", cur / thrValue);
        if (cur > thrValue) {
            worstValue = cur;
            wWorst = mid;
            keepIdx = -1;
        } else {
            dprint4("       Old best v2 ", bestValue2, " New best ", state->lexiCost()[1]);
            if (fabs(bestValue2 - state->lexiCost()[1]) / state->lexiCost()[1] < 0.01)
                break;
            bestValue = cur;
            bestValue2 = state->lexiCost()[1];
            wBest = mid;
            keepIdx = -1;
        }
        dprint4("BEST ", bestValue, " WORST ", worstValue);
    }
    dprint2("RETURNING ", wBest);
    weights_[0] = wBest; weights_[1] = (1.0 - wBest);
    initialized_.clear();
    for (mlcore::State* x : problem_->states()) {
        ((mllexi::LexiState * ) x)->resetCost(weights_, keepIdx);
        ((mllexi::LexiState * ) x)->setBestAction(nullptr);
    }
    solveLevel(s, 0, unsolved);

    return s->bestAction();
}

int LexiLAOStarSolver::expand(mllexi::LexiState* s, int level, mllexi::LexiState*& unsolved)
{
    if (level > 0 && solved_.find(s) == solved_.end()) {
        mllexi::LexiState* aux = nullptr;
        solveLevel(s, 0, aux);
    }

    /* making sure we are not using values from previous searches */
//    if (initialized_.insert(s).second) {
//        s->resetCost(weights_);
//        s->setBestAction(nullptr);
//    }

    if (!visited_.insert(s).second)  // state was already visited_
        return 0;

    if (s->deadEnd() || problem_->goal(s, 0))
        return 0;

    int cnt = 0;
    if (s->bestAction() == nullptr) {   // this means state has not been expanded
//        lexiBellmanUpdate(problem_, s, level);
        bellmanUpdate(problem_, s, weights_);
        return 1;
    } else {
        mlcore::Action* a = s->bestAction();
        for (mlcore::Successor sccr : problem_->transition(s, a, 0)) {
            cnt += expand((mllexi::LexiState *) sccr.su_state, level, unsolved);
            if (unsolved != nullptr) {
                return cnt;
            }
        }
    }

//    lexiBellmanUpdate(problem_, s, level);
    bellmanUpdate(problem_, s, weights_);
    return cnt;
}

double LexiLAOStarSolver::testConvergence(mllexi::LexiState* s, int level)
{
    double error = 0.0;

    if (s->deadEnd() || problem_->goal(s, 0))
        return 0.0;

    if (!visited_.insert(s).second)
        return 0.0;

    mlcore::Action* prevAction = s->bestAction();
    if (prevAction == nullptr) {
        // hasn't converged because a state in BPSG doesn't have an action ready
        return mdplib::dead_end_cost + 1;
    } else {
        for (mlcore::Successor sccr : problem_->transition(s, prevAction, 0))
            error =  std::max(error, testConvergence((mllexi::LexiState *) sccr.su_state, level));
    }

//    error = std::max(error, lexiBellmanUpdate(problem_, s, level));
    error = std::max(error, bellmanUpdate(problem_, s, weights_));
    if (prevAction == s->bestAction())
        return error;
    return mdplib::dead_end_cost + 2; // hasn't converged because the best action changed
}

void LexiLAOStarSolver::addSolved(mlcore::State* s)
{
    std::list<mlcore::State *> queue;
    queue.push_front(s);
    visited_.clear();
    while (!queue.empty()) {
        mlcore::State* cur = queue.front(); queue.pop_front();
        solved_.insert(cur);
        if (!visited_.insert(cur).second || problem_->goal(cur, 0))
            continue;
        mlcore::Action* a = cur->bestAction();
        mlcore::SuccessorsList sccrs = problem_->transition(cur, a, 0);
        for (mlcore::Successor su : sccrs)
            queue.push_front(su.su_state);
    }
}

}

