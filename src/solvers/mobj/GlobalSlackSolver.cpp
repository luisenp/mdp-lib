#include "../../../include/util/general.h"
#include "../../../include/solvers/Solver.h"
#include "../../../include/solvers/mobj/GlobalSlackSolver.h"

namespace mdplib_mobj_solvers
{

mlcore::Action* GlobalSlackSolver::solve(mlcore::State* s)
{
    mlmobj::MOState* unsolved = nullptr;

    double bestValue, worstValue, bestValue2;
    double wBest = 1.0, wWorst = 0.0;

    mlmobj::MOState* state = (mlmobj::MOState* ) s;

    std::vector<double> weights(problem_->size(), 0.0);

//    dprint1("Optima ");
//    std::vector<double> optima;
//    for (int i = 0; i < 3; i++) {
//        for (int j = 0; j < 3; j++)
//            weights[j] = 0.0;
//        weights[i] = 1.0;
//        resetStates(weights);
//        problem_->weights(weights);
//        internalSolver_->solve(s);
//        for (int j = 0; j < 3; j++)
//            std::cerr << state->mobjCost()[j] << " " << state->cost() << " - ";
//        optima.push_back(state->cost());
//        std::cerr << std::endl;
//    }

//    dprint1("\n Samples");
    weights[0] = 1.0;
    for (int i = 0; i < 100; i += 5.0)
        for (int j = 0; j < 1000; j += 50.0) {
            weights[1] = i; weights[2] = j;
            problem_->weights(weights);
            resetStates(weights);
            internalSolver_->solve(s);
            std::cerr << i << " " << j << " " << state->cost() << " ";
            for (int j = 0; j < 3; j++)
                std::cerr << state->mobjCost()[j] << " ";
            std::cerr << std::endl;
        }

    return s->bestAction();
}

mlcore::Action* GlobalSlackSolver::binarySearch(mlcore::State* s)
{
    mlmobj::MOState* unsolved = nullptr;

    double bestValue, worstValue, bestValue2;
    double wBest = 1.0, wWorst = 0.0;

    mlmobj::MOState* state = (mlmobj::MOState* ) s;

    std::vector<double> weights(problem_->size(), 0.0);
    weights[0] = 1.0; weights[1] = 0.0;
    problem_->weights(weights);
    internalSolver_->solve(s);
                dprint4("BEST ", state->mobjCost()[0], " ", state->mobjCost()[1]);
    bestValue = state->mobjCost()[0];
    bestValue2 = state->mobjCost()[1];
    double thrValue = (1.0 + problem_->slack()) * s->cost();
                dprint2("THRESHOLD ", thrValue);
    weights[0] = 0.0; weights[1] = 1.0;
    problem_->weights(weights);
    for (mlcore::State* x : problem_->states()) {
        ((mlmobj::MOState * ) x)->resetCost(weights, -1);
        ((mlmobj::MOState * ) x)->setBestAction(nullptr);
    }
    internalSolver_->solve(s);
    double cur = state->mobjCost()[0];
    worstValue = state->mobjCost()[0];
                dprint4("WORST ", state->mobjCost()[0], " ", state->mobjCost()[1]);

    if (cur <= thrValue)
        return s->bestAction();

    double lo = 0.0, hi = 1.0;
    int keepIdx = 1;
    for (int i = 0; i < 10; i++) {
        double mid = (wBest + wWorst) / 2;
        weights[0] = mid; weights[1] = (1.0 - mid);
        problem_->weights(weights);
        for (mlcore::State* x : problem_->states()) {
            ((mlmobj::MOState * ) x)->resetCost(weights, keepIdx);
            ((mlmobj::MOState * ) x)->setBestAction(nullptr);
        }
        internalSolver_->solve(s);

        cur = state->mobjCost()[0];;
        dprint4("WEIGHT ", mid, " RATIO ", cur / thrValue);
        if (cur > thrValue) {
            worstValue = cur;
            wWorst = mid;
            keepIdx = -1;
        } else {
            dprint4("       Old best v2 ", bestValue2, " New best ", state->mobjCost()[1]);
            if (fabs(bestValue2 - state->mobjCost()[1]) / state->mobjCost()[1] < 0.01)
                break;
            bestValue = cur;
            bestValue2 = state->mobjCost()[1];
            wBest = mid;
            keepIdx = -1;
        }
        dprint4("BEST ", bestValue, " WORST ", worstValue);
    }
    dprint2("RETURNING ", wBest);
    weights[0] = wBest; weights[1] = (1.0 - wBest);
    problem_->weights(weights);
    for (mlcore::State* x : problem_->states()) {
        ((mlmobj::MOState * ) x)->resetCost(weights, keepIdx);
        ((mlmobj::MOState * ) x)->setBestAction(nullptr);
    }
    internalSolver_->solve(s);

    return s->bestAction();
} // GlobalSlackSolver::binarySearch

} // namespace mdplib_mobj_solvers


