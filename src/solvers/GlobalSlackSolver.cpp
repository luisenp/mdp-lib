//#include "../../include/util/general.h"
//#include "../../include/solvers/solver.h"
//#include "../../include/solvers/GlobalSlackSolver.h"
//
//namespace mlsolvers
//{
//
//mlcore::Action* GlobalSlackSolver::solve(mlcore::State* s)
//{
//    mllexi::MOState* unsolved = nullptr;
//
//    double bestValue, worstValue, bestValue2;
//    double wBest = 1.0, wWorst = 0.0;
//
//    mllexi::MOState* state = (mllexi::MOState* ) s;
//
//    weights_[0] = 1.0; weights_[1] = 0.0;
//    internalSolver_->solveLevel(s, 0, unsolved);
//                dprint4("BEST ", state->lexiCost()[0], " ", state->lexiCost()[1]);
//    bestValue = state->lexiCost()[0];
//    bestValue2 = state->lexiCost()[1];
//    double thrValue = (1.0 + problem_->slack()) * s->cost();
//                dprint2("THRESHOLD ", thrValue);
//    weights_[0] = 0.0; weights_[1] = 1.0;
//    initialized_.clear();
//    for (mlcore::State* x : problem_->states()) {
//        ((mllexi::MOState * ) x)->resetCost(weights_, -1);
//        ((mllexi::MOState * ) x)->setBestAction(nullptr);
//    }
//    internalSolver_->solveLevel(s, 0, unsolved);
//    double cur = state->lexiCost()[0];
//    worstValue = state->lexiCost()[0];
//                dprint4("WORST ", state->lexiCost()[0], " ", state->lexiCost()[1]);
//
//    if (cur <= thrValue)
//        return s->bestAction();
//
//    double lo = 0.0, hi = 1.0;
//    int keepIdx = 1;
//    for (int i = 0; i < 10; i++) {
////        double slope = (worstValue - bestValue) / (wWorst - wBest);
////        double mid = wBest + (thrValue - bestValue) / slope;
//        double mid = (wBest + wWorst) / 2;
//        initialized_.clear();
//        weights_[0] = mid; weights_[1] = (1.0 - mid);
//        for (mlcore::State* x : problem_->states()) {
//            ((mllexi::MOState * ) x)->resetCost(weights_, keepIdx);
//            ((mllexi::MOState * ) x)->setBestAction(nullptr);
//        }
//        internalSolver_->solveLevel(s, 0, unsolved);
//
//        cur = state->lexiCost()[0];;
//        dprint4("WEIGHT ", mid, " RATIO ", cur / thrValue);
//        if (cur > thrValue) {
//            worstValue = cur;
//            wWorst = mid;
//            keepIdx = -1;
//        } else {
//            dprint4("       Old best v2 ", bestValue2, " New best ", state->lexiCost()[1]);
//            if (fabs(bestValue2 - state->lexiCost()[1]) / state->lexiCost()[1] < 0.01)
//                break;
//            bestValue = cur;
//            bestValue2 = state->lexiCost()[1];
//            wBest = mid;
//            keepIdx = -1;
//        }
//        dprint4("BEST ", bestValue, " WORST ", worstValue);
//    }
//    dprint2("RETURNING ", wBest);
//    weights_[0] = wBest; weights_[1] = (1.0 - wBest);
//    initialized_.clear();
//    for (mlcore::State* x : problem_->states()) {
//        ((mllexi::MOState * ) x)->resetCost(weights_, keepIdx);
//        ((mllexi::MOState * ) x)->setBestAction(nullptr);
//    }
//    internalSolver_->solveLevel(s, 0, unsolved);
//
//    return s->bestAction();
//}
//
//
//}
//
//
