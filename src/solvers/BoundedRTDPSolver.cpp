#include "../../include/solvers/BoundedRTDPSolver.h"

namespace mlsolvers
{

BoundedRTDPSolver::BoundedRTDPSolver(mlcore::Problem* problem,
                                     double epsilon,
                                     double upperBound,
                                     double tau,
                                     int maxTrials,
                                     int maxTime)
    : problem_(problem), epsilon_(epsilon), constantUpperBound_(upperBound),
      tau_(tau), maxTrials_(maxTrials), maxTime_(maxTime)
{ }


void BoundedRTDPSolver::trial(mlcore::State* s) {
    mlcore::State* tmp = s;
    std::list<mlcore::State*> visited;
    double accumulated_cost = 0.0;
    while (true) {
        if (problem_->goal(tmp))
            break;
        visited.push_front(tmp);
        this->bellmanUpdate(tmp);
        // Explore using the lower bound.
        mlcore::Action* a = lowerBoundGreedyPolicy_[tmp];
        accumulated_cost += problem_->cost(tmp, a);
        if (tmp->deadEnd() || accumulated_cost >= mdplib::dead_end_cost)
            break;

        if (ranOutOfTime()) {
            if (s->bestAction() == nullptr)
                this->bellmanUpdate(s);
            return;
        }

        tmp = sampleBiased(tmp, a, s);
        if (tmp == nullptr)
            break;
    }

    while (!visited.empty()) {
        tmp = visited.front();
        visited.pop_front();
        this->bellmanUpdate(tmp);
    }
}

void BoundedRTDPSolver::initializeUpperBound(mlcore::State* s) {
    if (problem_->goal(s))
        upperBounds_[s] = 0.0;
    else if (constantUpperBound_ > 0.0)
        upperBounds_[s] = constantUpperBound_;
    else
        upperBounds_[s] = 30.0; // TODO: Replace by some initialization
}

mlcore::State* BoundedRTDPSolver::sampleBiased(
        mlcore::State* s, mlcore::Action* a, mlcore::State* s0) {
    double B = 0.0;
    std::vector< std::pair<mlcore::State*, double> > statesAndScores;
    for (const mlcore::Successor& su : problem_->transition(s, a)) {
        double score =
            su.su_prob * (upperBounds_.at(su.su_state) - su.su_state->cost());
        statesAndScores.push_back(std::make_pair(su.su_state, score));
        B += score;
    }
    if ((upperBounds_.at(s) - s->cost()) == 0
            || B < (upperBounds_.at(s0) - s0->cost()) / tau_)
        return nullptr;
    double pick = kUnif_0_1(kRNG);
    double acc = 0;
    for (auto stateAndScore : statesAndScores) {
        acc += stateAndScore.second / B;
        if (acc >= pick) {
            return stateAndScore.first;
        }
    }
    assert(false);
    return nullptr;
}

double BoundedRTDPSolver::bellmanUpdate(mlcore::State* s) {
    double bestLowerBound = problem_->goal(s) ? 0.0 : mdplib::dead_end_cost;
    double bestUpperBound = problem_->goal(s) ? 0.0 : mdplib::dead_end_cost;
    bool hasAction = false;
    mlcore::Action* bestActionLowerBound = nullptr;
    mlcore::Action* bestActionUpperBound = nullptr;
    for (mlcore::Action* a : problem_->actions()) {
        if (!problem_->applicable(s, a))
            continue;
        hasAction = true;
        double lowerBoundQValueAction = 0.0;
        double upperBoundQValueAction = 0.0;
        for (const mlcore::Successor& su : problem_->transition(s, a)) {
            // state->cost() stores the lower bound.
            lowerBoundQValueAction += su.su_prob * su.su_state->cost();
            if (upperBounds_.count(su.su_state) == 0)
                initializeUpperBound(su.su_state);
            upperBoundQValueAction += su.su_prob * upperBounds_.at(su.su_state);
        }
        lowerBoundQValueAction =
            (lowerBoundQValueAction * problem_->gamma()) + problem_->cost(s, a);
        lowerBoundQValueAction = std::min(mdplib::dead_end_cost, lowerBoundQValueAction);
        upperBoundQValueAction =
            (upperBoundQValueAction * problem_->gamma()) + problem_->cost(s, a);
        upperBoundQValueAction = std::min(mdplib::dead_end_cost, upperBoundQValueAction);
        if (upperBoundQValueAction <= bestUpperBound) {
            bestUpperBound = upperBoundQValueAction;
            bestActionUpperBound = a;
        }
        if (lowerBoundQValueAction <= bestLowerBound) {
            bestLowerBound = lowerBoundQValueAction;
            bestActionLowerBound = a;
        }
    }
    if (!hasAction && bestLowerBound >= mdplib::dead_end_cost)
        s->markDeadEnd();
    s->setCost(bestLowerBound);
    upperBounds_[s] = bestUpperBound;
    lowerBoundGreedyPolicy_[s] = bestActionLowerBound;
    s->setBestAction(bestActionUpperBound);
    return bestUpperBound - bestLowerBound;
}

bool BoundedRTDPSolver::ranOutOfTime() {
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

mlcore::Action* BoundedRTDPSolver::solve(mlcore::State* s0) {
    int trials = 0;
    beginTime_ = std::chrono::high_resolution_clock::now();
    while (trials++ < maxTrials_) {
        trial(s0);
        if ((upperBounds_.at(s0) - s0->cost() < epsilon_) || ranOutOfTime())
            break;
    }
    return s0->bestAction();
}

void BoundedRTDPSolver::reset() {
    upperBounds_.clear();
    lowerBoundGreedyPolicy_.clear();
}

}

