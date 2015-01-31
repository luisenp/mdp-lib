#include "../../include/util/general.h"
#include "../../include/solvers/solver.h"
#include "../../include/solvers/LexiLAOStarSolver.h"

#include <ctime>

namespace mlsolvers
{

void LexiLAOStarSolver::solveLevel(mlcore::State* s, int level, mllexi::LexiState*& unsolved)
{
    mlsolvers::set_cost_v_eta = false;  // TODO: this is very ugly, but will do for now (Jan 2015)
    mllexi::LexiState* s0 = (mllexi::LexiState *) s;
    clock_t startTime = clock();
    int totalExpanded = 0;
    int countExpanded = 0;
    double error = mdplib::dead_end_cost;
    while (true) {
        do {
            visited.clear();
            unsolved = nullptr;
            countExpanded = expand(s0, level, unsolved);
            if (level > 0 && unsolved != nullptr) {
                return;
            }
            totalExpanded += countExpanded;

        } while (countExpanded != 0);

        while (true) {
            visited.clear();
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
    mdplib_debug = true;
    for (int i = 0; i < problem_->size(); i++) {
        dprint2("SOLVING LEVEL: ", i);
        mllexi::LexiState* unsolved = nullptr;
        solveLevel(s, i, unsolved);
        if (unsolved != nullptr) {
            dprint2("ERROR!!!!! NEED TO SOLVE ", unsolved);
            mllexi::LexiState* aux = nullptr;
            solveLevel(unsolved, 0, aux);
            i--;
        }
        dprint3("SOLVED LEVEL ", i, solved.size());
        dsleep(1000);
    }
    return s->bestAction();
}

int LexiLAOStarSolver::expand(mllexi::LexiState* s, int level, mllexi::LexiState*& unsolved)
{
    if (level > 0 && solved.find(s) == solved.end()) {
        mllexi::LexiState* aux = nullptr;
        dprint2("please solve ", s);
        solveLevel(s, 0, aux);
//        return 0;
    }

    if (!visited.insert(s).second)  // state was already visited
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
            cnt += expand((mllexi::LexiState *) sccr.su_state, level, unsolved);
            if (unsolved != nullptr) {
                return cnt;
            }
        }
    }
    lexiBellmanUpdate(problem_, s, level);
    return cnt;
}

double LexiLAOStarSolver::testConvergence(mllexi::LexiState* s, int level)
{
    double error = 0.0;

    if (s->deadEnd() || problem_->goal(s, 0))
        return 0.0;

    if (!visited.insert(s).second)
        return 0.0;

    mlcore::Action* prevAction = s->bestAction();
    if (prevAction == nullptr) {
        // hasn't converged because a state in BPSG doesn't have an action ready
        return mdplib::dead_end_cost + 1;
    } else {
        for (mlcore::Successor sccr : problem_->transition(s, prevAction, 0))
            error =  std::max(error, testConvergence((mllexi::LexiState *) sccr.su_state, level));
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
    visited.clear();
    while (!queue.empty()) {
        mlcore::State* cur = queue.front(); queue.pop_front();
        solved.insert(cur);
        if (!visited.insert(cur).second || problem_->goal(cur, 0))
            continue;
        mlcore::Action* a = cur->bestAction();
        mlcore::SuccessorsList sccrs = problem_->transition(cur, a, 0);
        for (mlcore::Successor su : sccrs)
            queue.push_front(su.su_state);
    }
}

}

