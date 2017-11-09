#include "../../include/solvers/VPIRTDPSolver.h"

namespace mlsolvers
{

VPIRTDPSolver::VPIRTDPSolver(mlcore::Problem* problem,
                                     double epsilon,
                                     int maxTrials)
    : problem_(problem), epsilon_(epsilon), maxTrials_(maxTrials), tau_(100.0)
{ }


void VPIRTDPSolver::trial(mlcore::State* s) {
    mlcore::State* tmp = s;
    std::list<mlcore::State*> visited;
    while (true) {
        if (problem_->goal(tmp))
            break;
        visited.push_front(tmp);
        this->bellmanUpdate(tmp);
        // Explore using the lower bound.
        mlcore::Action* a = lowerBoundGreedyPolicy_[tmp];
        if (tmp->deadEnd())
            break;
        tmp = sampleBiased(tmp, a);
        if (tmp == nullptr)
            break;
    }

    while (!visited.empty()) {
        tmp = visited.front();
        visited.pop_front();
        this->bellmanUpdate(tmp);
    }
}

void VPIRTDPSolver::initializeUpperBound(mlcore::State* s) {
    upperBounds_[s] = problem_->goal(s) ? 0.0 : 30.0;
}

mlcore::State*
VPIRTDPSolver::sampleBiased(mlcore::State* s, mlcore::Action* a) {
    double B = 0.0;
    std::vector< std::pair<mlcore::State*, double> > statesAndScores;
    for (const mlcore::Successor& su : problem_->transition(s, a)) {
        double score =
            su.su_prob * (upperBounds_[su.su_state] - su.su_state->cost());
        statesAndScores.push_back(std::make_pair(su.su_state, score));
        B += score;
    }
    if ((upperBounds_[s] - s->cost()) == 0
            || B < (upperBounds_[s] - s->cost()) / tau_)
        return nullptr;
    double pick = dis(gen);
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

double VPIRTDPSolver::bellmanUpdate(mlcore::State* s) {
    double bestLowerBound = problem_->goal(s) ? 0.0 : mdplib::dead_end_cost;
    double bestUpperBound = problem_->goal(s) ? 0.0 : mdplib::dead_end_cost;
    bool hasAction = false;
    mlcore::Action* bestActionLowerBound = nullptr;
    mlcore::Action* bestActionUpperBound = nullptr;
    for (mlcore::Action* a : problem_->actions()) {
        if (!problem_->applicable(s, a))
            continue;
        hasAction = true;
        double lowerBoundAction = 0.0;
        double upperBoundAction = 0.0;
        for (const mlcore::Successor& su : problem_->transition(s, a)) {
            // state->cost() stores the lower bound.
            lowerBoundAction += su.su_prob * su.su_state->cost();
            if (upperBounds_.count(su.su_state) == 0)
                initializeUpperBound(su.su_state);
            upperBoundAction += su.su_prob * upperBounds_[su.su_state];
        }
        lowerBoundAction =
            (lowerBoundAction * problem_->gamma()) + problem_->cost(s, a);
        lowerBoundAction = std::min(mdplib::dead_end_cost, lowerBoundAction);
        upperBoundAction =
            (upperBoundAction * problem_->gamma()) + problem_->cost(s, a);
        upperBoundAction = std::min(mdplib::dead_end_cost, upperBoundAction);
        if (upperBoundAction <= bestUpperBound) {
            bestUpperBound = upperBoundAction;
            bestActionUpperBound = a;
        }
        if (lowerBoundAction <= bestLowerBound) {
            bestLowerBound = lowerBoundAction;
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

mlcore::Action* VPIRTDPSolver::solve(mlcore::State* s0) {
    int trials = 0;
    while (trials++ < maxTrials_) {
        trial(s0);
        if (upperBounds_[s0] - s0->cost() < epsilon_)
            break;
    }
}

}
