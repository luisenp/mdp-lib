#include "../../include/solvers/SoftFLARESSolver.h"

#include <cmath>
                                                                                #include <chrono>

#include "../../include/MDPLib.h"

using namespace mlcore;
using namespace std;

namespace mlsolvers
{

SoftFLARESSolver::SoftFLARESSolver(Problem* problem,
                                   int maxTrials,
                                   double epsilon,
                                   double horizon,
                                   TransitionModifierFunction modifierFunction,
                                   double alpha,
                                   bool useProbsForDepth,
                                   bool optimal) :
        problem_(problem),
        maxTrials_(maxTrials),
        epsilon_(epsilon),
        horizon_(horizon),
        useProbsForDepth_(useProbsForDepth),
        modifierFunction_(modifierFunction),
        alpha_(alpha),
        optimal_(optimal) {
                                                                                //    tau_ = log((1 - alpha_) / alpha_) / (1 + horizon_);
//    tau_ = -2 * log(alpha_) / horizon_;
    tau_ = -2 * log((1-alpha_) / alpha_) / horizon_;
    modifierCache_.resize(horizon_ + 1);
    for (int i = 0; i <= horizon_; i++) {
        modifierCache_[i] = computeProbModfier(i);
                                                                                dprint("mod", i, modifierCache_[i]);
    }
}


void SoftFLARESSolver::trial(State* s) {
    State* currentState = s;
    list<State*> visited;
    double accumulated_cost = 0.0;
    while (!labeledSolved(currentState)) {
        if (problem_->goal(currentState))
            break;

        visited.push_front(currentState);
                                                                                double res = residual(problem_, currentState);
        bellmanUpdate(problem_, currentState);
                                                                                if (res < epsilon_ && residual(problem_, currentState) > epsilon_) {
                                                                                    cerr << "ooops!" << residual(problem_, currentState) << endl;
                                                                                }

        if (currentState->deadEnd()
                || accumulated_cost >= mdplib::dead_end_cost)
            break;

        mlcore::Action* greedy_action = greedyAction(problem_, currentState);
        accumulated_cost += problem_->cost(currentState, greedy_action);
                                                                                auto begin = std::chrono::high_resolution_clock::now();
        currentState = sampleSuccessor(currentState, greedy_action);
                                                                                auto end = std::chrono::high_resolution_clock::now();
                                                                                auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count();
                                                                                cnt_samples_++;
                                                                                total_time_samples_ += duration;

        if (currentState == nullptr)
            break;
    }

    while (!visited.empty()) {
        currentState = visited.front();
        visited.pop_front();
//        if (pick < computeProbModfier(currentState)) {
//            computeResidualDistances(currentState);
//        }

                                                                                auto begin = std::chrono::high_resolution_clock::now();
        computeResidualDistances(currentState);
                                                                                auto end = std::chrono::high_resolution_clock::now();
                                                                                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-begin).count();
                                                                                cnt_check_++;
                                                                                total_time_check_ += duration;
        if (kUnif_0_1(kRNG) < computeProbModfier(currentState)) {
            break;
        }
    }
}

double SoftFLARESSolver::computeProbModfier(mlcore::State* s) {
    if (s->residualDistance() == mdplib::no_distance)
        return 1.0;
    double distance = s->residualDistance();
    if (!useProbsForDepth_) {
        distance = modifierCache_[int(distance)];
    } else {
        return computeProbModfier(distance);
    }
}

double SoftFLARESSolver::computeProbModfier(double distance) {
    if (modifierFunction_ == kExponentialDecay) {
        return 1.0 / (1 + tau_ * exp(distance + 1)); // TODO: CHECK THIS EXPR.
    }
    if (modifierFunction_ == kStep) {
        if (2 * distance >= horizon_)
            return 0.0;
        return 1.0;
    }
    if (modifierFunction_ == kLogistic) {
        return 1.0 / (1 + exp(tau_ * (distance + 0.05 - horizon_ / 2.0)));
    }
    assert(false);
}

mlcore::State* SoftFLARESSolver::computeScores(
        mlcore::State* s, mlcore::Action* a,
        std::vector<double>& scores, double& totalScore) {
    totalScore = 0.0;
    scores.clear();
    for (mlcore::Successor sccr : problem_->transition(s, a)) {
        double score = sccr.su_prob * computeProbModfier(sccr.su_state);
        totalScore += score;
        scores.push_back(score);
    }
}


mlcore::State* SoftFLARESSolver::sampleSuccessor(mlcore::State* s,
                                                 mlcore::Action* a) {
    if (a == nullptr)
        return s;
    double pick = kUnif_0_1(kRNG);
    double acc = 0.0;
    for (mlcore::Successor sccr : problem_->transition(s, a)) {
        double score = sccr.su_prob * computeProbModfier(sccr.su_state);
        acc += score; // / totalScore;
        if (acc >= pick) {
            return sccr.su_state;
        }
    }
    return nullptr;
}

bool SoftFLARESSolver::labeledSolved(State* s) {
    return s->checkBits(mdplib::SOLVED);
}


double SoftFLARESSolver::computeNewDepth(Successor& su, double depth) {
    if (useProbsForDepth_) {
        return depth + log(su.su_prob);
    } else {
        return depth + 1;
    }
}


void SoftFLARESSolver::computeResidualDistances(State* s) {
    list<State*> open, closed;

    State* currentState = s;
    if (!currentState->checkBits(mdplib::SOLVED)) {
        open.push_front(currentState);
        currentState->depth(0.0);
    }

    bool rv = true;
    double lowestDepthHighResidual = horizon_;
    bool subgraphWithinSearchHorizon = true;
    while (!open.empty()) {
        State* currentState = open.front();
        open.pop_front();
        double depth = currentState->depth();
        if (depth > horizon_) {
            subgraphWithinSearchHorizon = false;
            continue;
        }

        if (problem_->goal(currentState))
            continue;

        closed.push_front(currentState);
        currentState->setBits(mdplib::CLOSED);

        Action* a = greedyAction(problem_, currentState);

        if (currentState->deadEnd())
            continue;

        if (residual(problem_, currentState) > epsilon_) {
            rv = false;
            lowestDepthHighResidual = std::min(lowestDepthHighResidual, depth);
        }

        for (Successor su : problem_->transition(currentState, a)) {
            State* next = su.su_state;
            if (!next->checkBits(mdplib::SOLVED)
                    && !next->checkBits(mdplib::CLOSED)) {
                open.push_front(next);
                next->depth(computeNewDepth(su, depth));
            }
        }
    }

    if (rv && subgraphWithinSearchHorizon) {
        for (mlcore::State* sc : closed) {
            sc->clearBits(mdplib::CLOSED);
            sc->setBits(mdplib::SOLVED);
            sc->depth(mdplib::no_distance);
        }
    } else {
        while (!closed.empty()) {
            State* state = closed.front();
            closed.pop_front();
            double depth = state->depth();
            state->clearBits(mdplib::CLOSED);
            if (depth <= lowestDepthHighResidual) {
                // Set the distance to the difference between the
                // depth at which a high residual was seen and the depth at
                // which this state was seen for the first time
                state->residualDistance(lowestDepthHighResidual - depth);
            }
                                                                                double res = residual(problem_, state);
            bellmanUpdate(problem_, state);
                                                                                if (res < epsilon_ && residual(problem_, state) > epsilon_) {
                                                                                    cerr << "ooops!" << residual(problem_, state) << endl;
                                                                                }
        }
    }
}


Action* SoftFLARESSolver::solve(State* s0) {
    int trials = 0;
                                                                                auto begin = std::chrono::high_resolution_clock::now();
    while (!labeledSolved(s0) && trials++ < maxTrials_) {
        trial(s0);
    }
                                                                                dprint("samples", cnt_samples_, double(total_time_samples_) / cnt_samples_);
                                                                                dprint("check", cnt_check_, double(total_time_check_) / cnt_check_);
                                                                                auto end = std::chrono::high_resolution_clock::now();
                                                                                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-begin).count();
                                                                                dprint("trials", trials, "total", duration);
    return s0->bestAction();
}

}   // namespace mlsolvers

