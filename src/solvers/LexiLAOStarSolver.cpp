#include "../../include/solvers/solver.h"
#include "../../include/solvers/LexiLAOStarSolver.h"

#include "../../include/util/general.h"

#include <ctime>

namespace mlsolvers
{

mlcore::Action* LexiLAOStarSolver::solve(mlcore::State* s)
{
    mllexi::LexiState *s0 = (mllexi::LexiState *) s;
    clock_t startTime = clock();
    int totalExpanded = 0;
    int countExpanded = 0;
    double error = mdplib::dead_end_cost;
    while (true) {
        do {
            visited.clear();
            countExpanded = expand(s0);
            totalExpanded += countExpanded;

            dprint2("TOTAL NODES EXPANDED ", countExpanded);

            clock_t currentTime = clock();
            if ((0.001 * (currentTime - startTime)) / CLOCKS_PER_SEC > timeLimit_)
                return s0->bestAction();

        } while (countExpanded != 0);

        while (true) {

            clock_t currentTime = clock();
            if ((0.001 * (currentTime - startTime)) / CLOCKS_PER_SEC > timeLimit_)
                return s0->bestAction();

            visited.clear();

            dprint1("******** CHECKING CONVERGENCE NOW!! ******** ");

            error = testConvergence(s0);

            dprint3("ERROR ", error, epsilon_);
            dprint2("VALUE ", s0->lexiCost()[0]);
//            dsleep(250);
            if (error < epsilon_)
                return s0->bestAction();
            if (error > mdplib::dead_end_cost) {
                break;  // BPSG changed, must expand tip nodes again
            }
        }
    }
}

int LexiLAOStarSolver::expand(mllexi::LexiState* s)
{
    explicitGraph_.insert(s);

    dprint3("EXPAND ", s, ((void *) s->bestAction()));
    if (!visited.insert(s).second)  // state was already visited
        return 0;

    if (s->deadEnd() || problem_->goal(s, 0))
        return 0;

    int cnt = 0;
    if (s->bestAction() == nullptr) {   // this means state has not been expanded
        lexiBellmanUpdate(problem_, s);
        return 1;
    } else {
        mlcore::Action* a = s->bestAction();
        for (mlcore::Successor sccr : problem_->transition(s, a, 0))
            cnt += expand((mllexi::LexiState *) sccr.su_state);
    }
    dprint2("UPDATING", s);
    lexiBellmanUpdate(problem_, s);
    dprint2("DONE!", s);
    return cnt;
}

double LexiLAOStarSolver::testConvergence(mllexi::LexiState* s)
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
            error =  std::max(error, testConvergence((mllexi::LexiState *) sccr.su_state));
    }

    error = std::max(error, lexiBellmanUpdate(problem_, s));
    if (prevAction == s->bestAction())
        return error;
    return mdplib::dead_end_cost + 2; // hasn't converged because the best action changed
}

}

