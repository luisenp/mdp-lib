#include "../../include/solvers/VPIRTDPSolver.h"
#include "../../include/util/math_utils.h"

namespace mlsolvers
{

VPIRTDPSolver::VPIRTDPSolver(mlcore::Problem* problem,
                                     double epsilon,
                                     int maxTrials,
                                     double alpha,
                                     double beta,
                                     double tau,
                                     double initialUpperBound)
    : problem_(problem),
      epsilon_(epsilon),
      maxTrials_(maxTrials),
      alpha_(alpha),
      beta_(beta),
      tau_(tau),
      initialUpperBound_(initialUpperBound)
{ }


void VPIRTDPSolver::trial(mlcore::State* s) {
    mlcore::State* tmp = s;
    std::list<mlcore::State*> visited;
    double accumulated_cost = 0.0;
    while (true) {
        if (problem_->goal(tmp))
            break;
        visited.push_front(tmp);
        this->bellmanUpdate(tmp);
                                                                                dprint(tmp, tmp->cost(), upperBounds_[tmp]);
        // Explore using the lower bound.
        mlcore::Action* a = lowerBoundGreedyPolicy_[tmp];
        accumulated_cost += problem_->cost(tmp, a);
        if (tmp->deadEnd())
            break;
        if (accumulated_cost > mdplib::dead_end_cost)
            break;
        tmp = sampleVPI(tmp, a);
        if (tmp == nullptr)
            break;


    }
                                                                                dprint("***************");

    while (!visited.empty()) {
        tmp = visited.front();
        visited.pop_front();
        this->bellmanUpdate(tmp);
    }
}

void VPIRTDPSolver::initializeUpperBound(mlcore::State* s) {
    upperBounds_[s] = problem_->goal(s) ? 0.0 : initialUpperBound_;
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
    if (mdplib_math::equal(upperBounds_[s], s->cost())
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
    mlcore::State* outcomeBRTDP = sampleBiasedBounds(s, sampledAction, B);
                                                                                dprint("sampling successor for ", s, sampledAction, "bounds", s->cost(), upperBounds_[s]);
                                                                                if (outcomeBRTDP)
                                                                                    dprint("brtdp", outcomeBRTDP, B, beta_);
    if (B > beta_) {
        return outcomeBRTDP;
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
            if (upperBounds_.count(su.su_state) == 0) {
                initializeUpperBound(su.su_state);
            }
            double stateContrib = su.su_prob
                * (upperBounds_[su.su_state] + su.su_state->cost()) / 2;
             qValue += stateContrib;
            statesContribQValues.back()[su.su_state] = stateContrib;
            statesProbs.back()[su.su_state] = su.su_prob;
        }
        qValue *= (problem_->gamma());
        qValue += problem_->cost(s, action);
        expectedQValuesGivenBounds[actionIndex] = qValue;
    }

    // Computing the myopic VPI for each outcome.
    mlcore::StateDoubleMap outcomeScores;
    double totalOutcomeScores = 0.0;
    for (const mlcore::Successor& su : problem_->transition(s, sampledAction)) {
        outcomeScores[su.su_state] = -std::numeric_limits<double>::max();
        assert(upperBounds_.count(su.su_state));
        double ub = upperBounds_[su.su_state];
        double lb = su.su_state->cost();
                                                                                dprint("  checking su", su.su_state, "lb", lb, "ub", ub);
        int actionIndex = -1;
        double PrSuGivenBestAction = statesProbs[indexBestAction][su.su_state];
        double contribSuBestAction =
            statesContribQValues[indexBestAction][su.su_state];
        double qValueRemAlpha =
            expectedQValuesGivenBounds[indexBestAction] - contribSuBestAction;
                                                                                dprint("  best: pr(o|alpha)", PrSuGivenBestAction, "tau(alpha,o)", qValueRemAlpha, "T(o|alpha)*E[Vo]", contribSuBestAction);
        double& outcomeScoreRef = outcomeScores[su.su_state];
        for (mlcore::Action* action : problem_->actions()) {
                                                                                dprint("    checking action", action);
            if (!problem_->applicable(s, action))
                continue;
            actionIndex++;
            double PrSuGivenAction = statesProbs[actionIndex][su.su_state];
            double contribSuAction =
                statesContribQValues[actionIndex][su.su_state];
            double qValueRemA =
                expectedQValuesGivenBounds[actionIndex] - contribSuAction;
            double outcomeScoreAction = -1;
                                                                                dprint("      action: pr(o|a)", PrSuGivenAction, "tau(a,o)", qValueRemA,  "T(o|a)*E[Vo]", contribSuAction);

            double normConst = ub == lb ? 1 : (ub - lb);
            if (mdplib_math::equal(PrSuGivenBestAction, PrSuGivenAction)) {
                outcomeScoreAction = std::max(0.0, qValueRemAlpha - qValueRemA);
                                                                                dprint("      case I ");
            } else if (mdplib_math::greaterThan(PrSuGivenBestAction,
                                                PrSuGivenAction)) {
                double deltaProb = PrSuGivenBestAction - PrSuGivenAction;
                double deltaRemQVal = qValueRemAlpha - qValueRemA;
                double z =
                    std::min( -deltaRemQVal / deltaProb, ub);
                z = std::max(z, lb);
                outcomeScoreAction =
                    (ub - z) * (deltaRemQVal + deltaProb * (z + ub) / 2);
                                                                                dprint("        pre-score", outcomeScoreAction, (ub-z), (deltaRemQVal + deltaProb * (z + ub) / 2));
                outcomeScoreAction /= normConst;
                                                                                dprint("      case II z ", z);
            } else {
                double deltaProb = PrSuGivenBestAction - PrSuGivenAction;
                double deltaRemQVal = qValueRemAlpha - qValueRemA;
                double z =
                    std::max( -deltaRemQVal / deltaProb, lb);
                z = std::min(ub, z);
                outcomeScoreAction =
                    (z - lb) * (deltaRemQVal + deltaProb * (lb + z) / 2);
                outcomeScoreAction /= normConst;
                                                                                dprint("      case III z ", z);
            }
                                                                                dprint("        score for successor and action ", su.su_state, action, outcomeScoreAction);
            assert(outcomeScoreAction >= 0);
            outcomeScoreRef = std::max(outcomeScoreRef, outcomeScoreAction);
        }
                                                                                dprint("    score: ", outcomeScoreRef);
        totalOutcomeScores += outcomeScoreRef;
    }
                                                                                dprint("  TOTAL_SCORE: ", totalOutcomeScores);
    if (totalOutcomeScores < mdplib::epsilon) {
        double pick = dis(gen);
        if (pick < alpha_)
            return outcomeBRTDP;
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
                                                                                dprint("******", s0->cost(), upperBounds_[s0], "******");
        if (upperBounds_[s0] - s0->cost() < epsilon_)
            break;
    }
}

}
