#include "../../include/util/general.h"
#include "../../include/solvers/solver.h"
#include "../../include/solvers/LexiLAOStarSolver.h"

#include <ctime>

namespace mlsolvers
{

void LexiLAOStarSolver::solveLevel(mlcore::State* s, int level, mllexi::MOState*& unsolved)
{
    mllexi::MOState* s0 = (mllexi::MOState *) s;
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
    mllexi::MOState* unsolved = nullptr;
    solveLevel(s, problem_->size() - 1, unsolved);
    return s->bestAction();
}

int LexiLAOStarSolver::expand(mllexi::MOState* s, int level, mllexi::MOState*& unsolved)
{
    if (level > 0 && solved_.find(s) == solved_.end()) {
        mllexi::MOState* aux = nullptr;
        solveLevel(s, 0, aux);
    }

    if (!visited_.insert(s).second)  // state was already visited_
        return 0;

    if (s->deadEnd() || problem_->goal(s, 0))
        return 0;

    int cnt = 0;
    if (s->bestAction() == nullptr) {   // this means state has not been expanded
        lexiBellmanUpdate(problem_, s, level);
        return 1;
    } else {
        mlcore::Action* a = s->bestAction();
        for (mlcore::Successor sccr : problem_->transition(s, a, 0)) {
            cnt += expand((mllexi::MOState *) sccr.su_state, level, unsolved);
            if (unsolved != nullptr) {
                return cnt;
            }
        }
    }

    lexiBellmanUpdate(problem_, s, level);
    return cnt;
}

double LexiLAOStarSolver::testConvergence(mllexi::MOState* s, int level)
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
            error =  std::max(error, testConvergence((mllexi::MOState *) sccr.su_state, level));
    }

    error = std::max(error, lexiBellmanUpdate(problem_, s, level));
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

