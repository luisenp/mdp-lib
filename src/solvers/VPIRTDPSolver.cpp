#include "../../include/solvers/VPIRTDPSolver.h"
#include "../../include/util/math_utils.h"

namespace mlsolvers
{

VPIRTDPSolver::VPIRTDPSolver(mlcore::Problem* problem,
                                     double epsilon,
                                     int maxTrials)
    : problem_(problem),
      epsilon_(epsilon),
      maxTrials_(maxTrials),
      alpha_(0.01),
      beta_(25),
      tau_(100.0)
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
        tmp = sampleVPI(tmp, a);
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
VPIRTDPSolver::sampleBiasedBounds(mlcore::State* s,
                                  mlcore::Action* a,
                                  double& B) {
    B = 0.0;
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

mlcore::State*
VPIRTDPSolver::sampleVPI(mlcore::State* s, mlcore::Action* sampledAction) {
    double B;
    mlcore::State* outcomeBRTD = sampleBiasedBounds(s, sampledAction, B);
    if (B > beta_) {
        return outcomeBRTD;
    }
    // An action chosen according to the upper bound.
    mlcore::Action* bestAction = s->bestAction();

    // Pre-computing E[Qa | bounds] for all actions.
    // We also cache P(s'|s,action) and (s'|s,action) * (UB(s') - LB(s')) / 2.
    std::vector<double> expectedQValuesGivenBounds;
    std::vector<mlcore::StateDoubleMap> statesContribQValues;
    std::vector<mlcore::StateDoubleMap> statesProbs;
    int actionIndex = -1;
    int indexBestAction = -1;
    for (mlcore::Action* action : problem_->actions()) {
        if (!problem_->applicable(s, action))
            continue;
        actionIndex++;
        if (action == bestAction)
            indexBestAction = actionIndex;
        double qValue = 0.0;
        expectedQValuesGivenBounds.push_back(0.0);
        statesContribQValues.push_back(mlcore::StateDoubleMap ());
        statesProbs.push_back(mlcore::StateDoubleMap ());
        for (const mlcore::Successor& su : problem_->transition(s, action)) {
            double stateContrib =
                su.su_prob * (upperBounds_[su.su_state] - su.su_state->cost());
            qValue += stateContrib;
            statesContribQValues.back()[su.su_state] = stateContrib / 2;
            statesProbs.back()[su.su_state] = su.su_prob;
        }
        qValue *= (problem_->gamma() / 2);
        qValue += problem_->cost(s, action);
        expectedQValuesGivenBounds[actionIndex] = qValue;
    }

    // Computing the myopic VPI for each outcome.
    mlcore::StateDoubleMap outcomeScores;
    double totalOutcomeScores = 0.0;
    for (const mlcore::Successor& su : problem_->transition(s, sampledAction)) {
        outcomeScores[su.su_state] = -std::numeric_limits<double>::max();
        int actionIndex = -1;
        double PrSuGivenBestAction = statesProbs[indexBestAction][su.su_state];
        double contribSuBestAction =
            statesContribQValues[indexBestAction][su.su_state];
        double qValueRemAlpha =
            expectedQValuesGivenBounds[indexBestAction] - contribSuBestAction;
        double& outcomeScoreRef = outcomeScores[su.su_state];
        for (mlcore::Action* action : problem_->actions()) {
            if (!problem_->applicable(s, action))
                continue;
            actionIndex++;
            double PrSuGivenAction = statesProbs[actionIndex][su.su_state];
            double contribSuAction =
                statesContribQValues[actionIndex][su.su_state];
            double qValueRemA =
                expectedQValuesGivenBounds[actionIndex] - contribSuAction;
            double ub = upperBounds_[su.su_state];
            double lb = su.su_state->cost();
            double outcomeScoreAction = -1;
            if (mdplib_math::equal(PrSuGivenAction, PrSuGivenBestAction)) {
                outcomeScoreAction =
                    std::max(0.0, (qValueRemA - qValueRemAlpha) * (ub - lb));
            } else if (mdplib_math::greaterThan(PrSuGivenAction,
                                                PrSuGivenBestAction)) {
                double deltaProb = PrSuGivenAction - PrSuGivenBestAction;
                double z =
                    std::min((qValueRemAlpha - qValueRemA) / deltaProb, ub);
                outcomeScoreAction =
                    (ub - z) * (qValueRemA - qValueRemAlpha
                                + deltaProb * (z + ub) / 2);
            } else {
                double deltaProb = PrSuGivenAction - PrSuGivenBestAction;
                double z =
                    std::max((qValueRemAlpha - qValueRemA) / deltaProb, lb);
                outcomeScoreAction =
                    (z - lb) * (qValueRemA - qValueRemAlpha
                                + deltaProb * (lb + z) / 2);
            }
                                                                                if (outcomeScoreAction < 0) {
                                                                                    dprint4("best", PrSuGivenBestAction,  contribSuBestAction, qValueRemAlpha);
                                                                                    dprint4("action", PrSuGivenAction,  contribSuAction, qValueRemA);
                                                                                    dprint1(outcomeScoreAction);
                                                                                }
            assert(outcomeScoreAction >= 0);
            outcomeScoreRef = std::max(outcomeScoreRef, outcomeScoreAction);
        }
        totalOutcomeScores += outcomeScoreRef;
    }
    if (totalOutcomeScores < mdplib::epsilon) {
        double pick = dis(gen);
        if (pick < alpha_)
            return sampleBiasedBounds(s, sampledAction, B);
        return nullptr;
    }
    double pick = dis(gen);
    double acc = 0.0;
    for (const mlcore::Successor& su : problem_->transition(s, sampledAction)) {
        acc += outcomeScores[su.su_state] / totalOutcomeScores;
        if (acc >= pick) {
            return su.su_state;
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
