#include "../../include/solvers/SoftFLARESSolver.h"

#include <chrono>
#include <cmath>

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
                                   DistanceFunction distanceFunction,
                                   HorizonFunction horizonFunction,
                                   double alpha,
                                   bool useProbsForDepth,
                                   bool noLabeling,
                                   bool optimal,
                                   double psi,
                                   int maxTime) :
        problem_(problem),
        maxTrials_(maxTrials),
        epsilon_(epsilon),
        horizon_(horizon),
        modifierFunction_(modifierFunction),
        distanceFunction_(distanceFunction),
        horizonFunction_(horizonFunction),
        alpha_(alpha),
        useProbsForDepth_(useProbsForDepth),
        noLabeling_(noLabeling),
        optimal_(optimal),
        maxTime_(maxTime),
        psi_(psi),
        useCache_(true) {
                                                                                dprint("SOFT-FLARES",
                                                                                       "horizon", horizon_,
                                                                                       "alpha", alpha_,
                                                                                       "maxTime", maxTime_);
    beta_ = 1 - alpha_;
//                                                                                beta_ = 0.5;
    if (modifierFunction_ == kLogistic) {
        double num = (1- alpha_) * beta_;
        double den = (1 - beta_) * alpha_;
        tau_ = -log(num / den) / horizon_;
    } else if (modifierFunction_ == kExponential) {
        tau_ = log(beta_ / alpha_) / horizon_;
    } else if (modifierFunction_ == kLinear) {
        tau_ = (beta_ - alpha_) / horizon_;
    }
    modifierCache_.resize(horizon_ + 1);
    for (int i = 0; i <= horizon_; i++) {
        modifierCache_[i] = computeProbUnlabeled(i);
                                                                                dprint(i, modifierCache_[i]);
    }
}

void SoftFLARESSolver::trial(State* s) {
    State* currentState = s;
    list<State*> visited;
    double accumulated_cost = 0.0;

    while (true) {
//                                                                                if (problem_->goal(currentState))
//                                                                                    dprint("GOAL!!", accumulated_cost);
        if (problem_->goal(currentState))
            break;

        visited.push_front(currentState);
        bellmanUpdate(problem_, currentState);

        if (currentState->deadEnd()
                || accumulated_cost >= mdplib::dead_end_cost)
            break;

        if (ranOutOfTime())
            return;

        mlcore::Action* greedy_action = greedyAction(problem_, currentState);
        accumulated_cost += problem_->cost(currentState, greedy_action);

        currentState = noLabeling_ ?
            randomSuccessor(problem_, currentState, greedy_action):
            sampleSuccessor(currentState, greedy_action);

        if (currentState == nullptr) {
            break;
        }
    }

    if (noLabeling_) return;
                                                                                dprint("COST ", accumulated_cost);

    while (!visited.empty()) {
        currentState = visited.front();
        visited.pop_front();
        computeResidualDistances(currentState);

        if (!labeledSolved(currentState))
            break;
    }
}

double SoftFLARESSolver::computeProbUnlabeled(mlcore::State* s) {
    double distance = s->residualDistance();
    if (distance < 0)
        return 1.0;
    if (useCache_) {
        return modifierCache_[int(distance)];
    } else {
        return computeProbUnlabeled(distance);
    }
}

double SoftFLARESSolver::computeProbUnlabeled(double distance) {
    assert(distance >= 0);

    if (distance == kInfiniteDistance_)
        return 0.0;

    if (horizon_ == 0) {
        return 1.0 - beta_;
    }
    if (modifierFunction_ == kStep) {
        if (distance >= 0)
            return 1.0 - beta_;
        return 1.0 - alpha_;
    }
    if (modifierFunction_ == kLogistic) {
        double C = (1 - alpha_) / alpha_;
        return 1.0 - 1.0 / (1 + C * exp(tau_ * distance));
    }
    if (modifierFunction_ == kExponential) {
        return 1.0 - alpha_ * exp(tau_ * distance);
    }
    if (modifierFunction_ == kLinear) {
        return 1.0 - (tau_ * distance + alpha_);
    }
    assert(false);
}

double SoftFLARESSolver::
computeNewDepth(Successor& su, double depth, double maxProbSuccessor) {
    if (distanceFunction_ == kTrajProb) {
        return depth - log2(su.su_prob);
    } else if (distanceFunction_ == kStepDist) {
        return depth + 1;
    } else if (distanceFunction_ == kPlaus) {
        double kappa = int(floor(log2(maxProbSuccessor) - log2(su.su_prob)));
        return depth + kappa;
    }
}

mlcore::State* SoftFLARESSolver::sampleSuccessor(mlcore::State* s,
                                                 mlcore::Action* a) {
    if (a == nullptr)
        return s;

    double pick = kUnif_0_1(kRNG);
    double acc = 0.0;
    int index = 0;
    for (mlcore::Successor sccr : problem_->transition(s, a)) {
        double p = computeProbUnlabeled(sccr.su_state) * sccr.su_prob;
        acc += p;
        if (acc >= pick) {
            return sccr.su_state;
        }
    }
    return nullptr;
}

bool SoftFLARESSolver::labeledSolved(State* s) {
    if (s->checkBits(mdplib::SOLVED))
        return true;
    bool labeled = (kUnif_0_1(kRNG) > computeProbUnlabeled(s));
    return labeled;
}

double SoftFLARESSolver::sampleEffectiveHorizon() {
    if (horizonFunction_ == kFixed) {
        return horizon_;
    }
    if (horizonFunction_ == kBernoulli) {
        if (kUnif_0_1(kRNG) > psi_) {
            return horizon_;
        } else {
            return kInfiniteDistance_;
        }
    }
}

void SoftFLARESSolver::computeResidualDistances(State* s) {
    list<State*> open, closed;

    State* currentState = s;
    if (!currentState->checkBits(mdplib::SOLVED)) {
        open.push_front(currentState);
        currentState->depth(0.0);
    }

    bool should_label = true;
    bool subgraphWithinSearchHorizon = true;
    double effectiveHorizon = sampleEffectiveHorizon();

    while (!open.empty()) {
        State* currentState = open.front();
        open.pop_front();
        double depth = currentState->depth();
        if (depth > 2 * effectiveHorizon) {
            subgraphWithinSearchHorizon = false;
            continue;
        }

        if (problem_->goal(currentState))
            continue;
//                                                                                if (ranOutOfTime()) {
//                                                                                    dprint("ran out of time", closed.size());
//                                                                                }
        if (ranOutOfTime())
            return;

        closed.push_front(currentState);
        currentState->setBits(mdplib::CLOSED);

        Action* a = greedyAction(problem_, currentState);

        if (currentState->deadEnd())
            continue;

        if (residual(problem_, currentState) > epsilon_) {
            should_label = false;
        }

        double maxProbSuccessor = 0.0;
        if (distanceFunction_ == kPlaus) {
            for (Successor& su : problem_->transition(currentState, a)) {
                maxProbSuccessor = std::max(maxProbSuccessor, su.su_prob);
            }
        }
        for (Successor& su : problem_->transition(currentState, a)) {
            State* next = su.su_state;
            if ( (!labeledSolved(next)
                        || effectiveHorizon == kInfiniteDistance_)
                    && !next->checkBits(mdplib::CLOSED)) {
                open.push_front(next);
                next->depth(computeNewDepth(su, depth, maxProbSuccessor));
            } else if (!(next->checkBits(mdplib::SOLVED)
                            || next->checkBits(mdplib::CLOSED))) {
                // If this happens, the state was skipped only due to
                // soft-labeling. Thus, there must be parts of the subgraph
                // that are outside of the horizon
                subgraphWithinSearchHorizon = false;
            }
        }
    }
//                                                                                dprint("closed ", closed.size());

    if (should_label) {
        for (mlcore::State* state : closed) {
            state->clearBits(mdplib::CLOSED);
            if (subgraphWithinSearchHorizon) {
                state->setBits(mdplib::SOLVED);
            } else {
                                                                                assert(effectiveHorizon != kInfiniteDistance_);
                double depth = state->depth();
                if (depth <= effectiveHorizon) {
                    state->residualDistance(effectiveHorizon - depth);
                                                                                dprint(state->residualDistance());
                }
            }
        }
    } else {
        while (!closed.empty()) {
            State* state = closed.front();
            closed.pop_front();
            state->clearBits(mdplib::CLOSED);
            bellmanUpdate(problem_, state);
        }
    }
}


bool SoftFLARESSolver::moreTrials(
        mlcore::State* s,
        int trialsSoFar,
        std::chrono::time_point<std::chrono::high_resolution_clock> startTime) {
    if (trialsSoFar >= maxTrials_)
        return false;
    if (optimal_) {
        return !s->checkBits(mdplib::SOLVED);
    }
    if (maxTime_ <= -1) {
//                                                                                bool tmp = !labeledSolved(s) && trialsSoFar < maxTrials_;
//                                                                                dprint("check trial", tmp);
//                                                                                return tmp;

        return !labeledSolved(s) && trialsSoFar < maxTrials_;
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::
        duration_cast<std::chrono::milliseconds>(endTime-startTime).count();
    return (duration < maxTime_);
}

bool SoftFLARESSolver::ranOutOfTime() {
    // Checking if it ran out of time
    if (maxTime_ > -1) {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto timeElapsed = std::chrono::duration_cast<
            std::chrono::milliseconds>(endTime - beginTime_).count();
        if (timeElapsed > maxTime_)
            return true;
    }
    return false;
}

Action* SoftFLARESSolver::solve(State* s0) {
    int trials = 0;
    beginTime_ = std::chrono::high_resolution_clock::now();
    while (moreTrials(s0, trials, beginTime_)) {
        trial(s0);
        trials++;
//                                                                                dprint(s0->residualDistance(), noLabeling_);
//                                                                                dprint("****************** trial ended", trials);
    }
    return greedyAction(problem_, s0);
}

}   // namespace mlsolvers

