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
                                     double initialUpperBound,
                                     bool vanillaSample)
    : problem_(problem),
      epsilon_(epsilon),
      maxTrials_(maxTrials),
      alpha_(alpha),
      beta_(beta),
      tau_(tau),
      initialUpperBound_(initialUpperBound),
      vanillaSample_(vanillaSample)
{ }


void VPIRTDPSolver::trial(mlcore::State* s) {
    mlcore::State* tmp = s;
    std::list<mlcore::State*> visited;
    double accumulated_cost = 0.0;
    while (true) {
        if (problem_->goal(tmp))
            break;
        visited.push_front(tmp);
                                                                                tmp->setBits(mdplib::VISITED);
        this->bellmanUpdate(tmp);
                                                                                dprint(tmp, tmp->cost(), upperBounds_[tmp]);
        // Explore using the lower bound.
        mlcore::Action* a = lowerBoundGreedyPolicy_[tmp];
        accumulated_cost += problem_->cost(tmp, a);
        if (tmp->deadEnd())
            break;
        if (accumulated_cost >= mdplib::dead_end_cost)
            break;
        tmp = vanillaSample_ ?
            randomSuccessor(problem_, tmp, a) : sampleVPIOld(tmp, a);
        if (tmp == nullptr)
            break;
    }
                                                                                dprint("***************");

    while (!visited.empty()) {
        tmp = visited.front();
        visited.pop_front();
        tmp->clearBits(mdplib::VISITED);
        this->bellmanUpdate(tmp);
                                                                                dprint("  ", tmp, tmp->cost(), upperBounds_[tmp]);
    }
}

void VPIRTDPSolver::initializeUpperBound(mlcore::State* s) {
    upperBounds_[s] = problem_->goal(s) ? 0.0 : initialUpperBound_;
}

mlcore::State*
VPIRTDPSolver::sampleBiasedBounds(mlcore::State* s,
                                  mlcore::Action* a) {
    double B = 0.0;
    std::vector< std::pair<mlcore::State*, double> > statesAndScores;
    for (const mlcore::Successor& su : problem_->transition(s, a)) {
                                                                                if (su.su_state->checkBits(mdplib::VISITED))
                                                                                    continue;
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
    // The current policy can achieve at worst this value. This method finds
    // the outcome that can improve this upper bound the most, assuming we
    // had perfect information about the outcome
    double VhState = upperBounds_[s];

    // Pre-computing Qh(s,a) for all actions (i.e., the upper bound on their
    // Q-values). Also cache P(s'|s,action) * Vh(s) and P(s'|s,action) for all
    // of their outcomes.
    // These are stored in statesContribQValues and statesProbs, respectively
    std::vector<double> QhActions;
    std::vector<mlcore::StateDoubleMap> statesContribQValues;
    std::vector<mlcore::StateDoubleMap> statesProbs;
    int actionIndex = -1;
    for (mlcore::Action* action : problem_->actions()) {
        if (!problem_->applicable(s, action))
            continue;
        actionIndex++;
        double qValue = 0.0;
        statesContribQValues.push_back(mlcore::StateDoubleMap ());
        statesProbs.push_back(mlcore::StateDoubleMap ());
        for (const mlcore::Successor& su : problem_->transition(s, action)) {
            if (upperBounds_.count(su.su_state) == 0) {
                initializeUpperBound(su.su_state);
            }
            double stateContrib = su.su_prob * upperBounds_[su.su_state];
            qValue += stateContrib;
            // The convention of this library is that a state can appear in
            // multiple outcomes. Thus we need to sum over the total
            // contribution of a state across multiple outcomes.
            if (statesProbs.back().count(su.su_state) == 0) {
                statesContribQValues.back()[su.su_state] = 0.0;
                statesProbs.back()[su.su_state] = 0.0;
            }
            statesContribQValues.back()[su.su_state] += stateContrib;
            statesProbs.back()[su.su_state] += su.su_prob;
        }
        qValue *= (problem_->gamma());
        qValue += problem_->cost(s, action);
        QhActions.push_back(qValue);
    }
                                                                                /*{
                                                                                    actionIndex = -1;
                                                                                    dprint("vpi computation for", s, "bestAction", sampledAction, "bounds", s->cost(), upperBounds_[s]);
                                                                                    for (mlcore::Action* action : problem_->actions()) {
                                                                                        if (!problem_->applicable(s, action))
                                                                                            continue;
                                                                                        actionIndex++;
                                                                                        dprint("    checking successors for ", action, "Qh(s,a)", QhActions[actionIndex]);
                                                                                        double totalP = 0.0;
                                                                                        double EQcalc = problem_->cost(s, action);
                                                                                        for (auto stateProb : statesProbs[actionIndex]) {
                                                                                            totalP += stateProb.second;
                                                                                            EQcalc += statesContribQValues[actionIndex][stateProb.first];
                                                                                            dprint("          ", stateProb.first, "p(o|a)", stateProb.second,
                                                                                                   "lb", stateProb.first->cost(), "ub", upperBounds_[stateProb.first],
                                                                                                   "contrib", statesContribQValues[actionIndex][stateProb.first]);
                                                                                        }
                                                                                        assert(mdplib_math::equal(totalP, 1.0));
                                                                                        assert(mdplib_math::equal(EQcalc, QhActions[actionIndex]));
                                                                                    }
                                                                                }*/
    // Computing the myopic VPI for each successor state
    mlcore::StateDoubleMap successorVPIs;
    double totalVPI = 0.0;
    for (const mlcore::Successor& su : problem_->transition(s, sampledAction)) {
        if (successorVPIs.count(su.su_state) != 0) {
            // This successor appeared associated to another outcome, so
            // VPI was already computed for it
            continue;
        }
        successorVPIs[su.su_state] = -std::numeric_limits<double>::max();
        assert(upperBounds_.count(su.su_state));
        double upBound = upperBounds_[su.su_state];
        double lowBound = su.su_state->cost();
        double normConst = mdplib_math::equal(upBound, lowBound) ?
            1.0 : (upBound - lowBound);
        double & vpiSuccessor = successorVPIs[su.su_state];
        actionIndex = -1;
        // Computing what the maximum cost of the policy would be
        // if we have perfect information about the successor's value
                                                                                dprint("  checking outcome VPI", lowBound, upBound);
        for (mlcore::Action* action : problem_->actions()) {
            if (!problem_->applicable(s, action))
                continue;
            actionIndex++;
            double QhRemaining = QhActions[actionIndex]
                - statesContribQValues[actionIndex][su.su_state];
            double PrSuGivenAction = statesProbs[actionIndex][su.su_state];
            if (mdplib_math::equal(PrSuGivenAction, 0.0)) {
                // Not a successor of this action, can't improve its bounds.
                continue;
            }
                                                                                dprint("    QhRemaining", QhRemaining, "p(o|a)", PrSuGivenAction);
            // V*(su) = z is the point below which the policy is guaranteed
            // to be better than the current policy
            double z = std::min((VhState - QhRemaining) / PrSuGivenAction,
                                upBound);
            if (mdplib_math::lessThan(z, lowBound)) {
                // No value of V*(su) that can improve the current policy
                vpiSuccessor = 0.0;
            } else {
                double vpiSuAction = VhState -
                    (QhRemaining + PrSuGivenAction * (z + lowBound) / 2);
                vpiSuAction /= normConst;
                                                                                    dprint("    z", z, "vpiSuAction", vpiSuAction);
                vpiSuccessor = std::max(vpiSuccessor, vpiSuAction);
            }
        }
        assert(vpiSuccessor >= 0.0);
        totalVPI += vpiSuccessor;
    }

    if (totalVPI < mdplib::epsilon) {
        double pick = dis(gen);
        if (pick < alpha_)
            return sampleBiasedBounds(s, sampledAction);
        return nullptr;
    }
    double pick = dis(gen);
    double acc = 0.0;
    for (const mlcore::Successor& su : problem_->transition(s, sampledAction)) {
        acc += successorVPIs[su.su_state] / totalVPI;
        if (acc >= pick) {
                                                                                dprint("sampled", su.su_state);
                                                                                dprint("*******************");
            return su.su_state;
        }
    }
    assert(false);
    return nullptr;
}

mlcore::State*
VPIRTDPSolver::sampleVPIOld(mlcore::State* s, mlcore::Action* sampledAction) {
    // An action chosen according to the upper bound.
    mlcore::Action* bestAction = lowerBoundGreedyPolicy_[s];

    // Pre-computing E[Qa | bounds] for all actions.
    // Also cache P(s'|s,action) and (s'|s,action) * P(UB(s') - LB(s')) / 2.
    // These are stored in statesProbs and statesContribQValues, respectively
                                                                                bool show = (dis(gen) < 0.0000001);
                                                                                if (show) mdplib_debug = true;
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
            // Skip VPI computation early on when bounds gap is large
            if (actionIndex == indexBestAction
                && (upperBounds_[su.su_state] - su.su_state->cost()) > beta_ ) {
                                                                                if (show) mdplib_debug = false;
                return sampleBiasedBounds(s, sampledAction);
            }
            double stateContrib = su.su_prob
                * (upperBounds_[su.su_state] + su.su_state->cost()) / 2;
            qValue += stateContrib;
            // The convention of this library is that a state can appear in
            // multiple outcomes. Thus we need to account for the total
            // contribution of a state across multiple outcomes.
            if (statesProbs.back().count(su.su_state) == 0) {
                statesContribQValues.back()[su.su_state] = 0.0;
                statesProbs.back()[su.su_state] = 0.0;
            }
            statesContribQValues.back()[su.su_state] += stateContrib;
            statesProbs.back()[su.su_state] += su.su_prob;
        }
        qValue *= (problem_->gamma());
        qValue += problem_->cost(s, action);
        expectedQValuesGivenBounds[actionIndex] = qValue;
    }
                                                                                if (show) {
                                                                                    actionIndex = -1;
                                                                                    dprint("vpi computation for", s, "bestAction", sampledAction, "bounds", s->cost(), upperBounds_[s]);
                                                                                    for (mlcore::Action* action : problem_->actions()) {
                                                                                        if (!problem_->applicable(s, action))
                                                                                            continue;
                                                                                        actionIndex++;
                                                                                        dprint("    checking successors for ", action, "E[Qa|bounds]", expectedQValuesGivenBounds[actionIndex]);
                                                                                        double totalP = 0.0;
                                                                                        double EQcalc = problem_->cost(s, action);
                                                                                        for (auto stateProb : statesProbs[actionIndex]) {
                                                                                            totalP += stateProb.second;
                                                                                            EQcalc += statesContribQValues[actionIndex][stateProb.first];
                                                                                            dprint("          ", stateProb.first, "p(o|a)", stateProb.second,
                                                                                                   "lb", stateProb.first->cost(), "ub", upperBounds_[stateProb.first],
                                                                                                   "contrib", statesContribQValues[actionIndex][stateProb.first]);
                                                                                        }
                                                                                        assert(mdplib_math::equal(totalP, 1.0));
                                                                                        assert(mdplib_math::equal(EQcalc, expectedQValuesGivenBounds[actionIndex]));
                                                                                    }
                                                                                }


    // Computing the myopic VPI for each successor state.
    mlcore::StateDoubleMap successorVPIs;
    double totalVPI = 0.0;
    for (const mlcore::Successor& su : problem_->transition(s, sampledAction)) {
        if (successorVPIs.count(su.su_state) != 0) {
            // This successor appeared associated to another outcome, so
            // VPI was already computed for it.
            continue;
        }
                                                                                /*if (su.su_state->checkBits(mdplib::VISITED))
                                                                                    continue;*/
        successorVPIs[su.su_state] = -std::numeric_limits<double>::max();
        assert(upperBounds_.count(su.su_state));
        double upBound = upperBounds_[su.su_state];
        double lowBound = su.su_state->cost();
        // Quantities suffixed by alpha refer to the current best action
        double PrSuGivenAlpha = statesProbs[indexBestAction][su.su_state];
        double contribSuQAlpha =
            statesContribQValues[indexBestAction][su.su_state];
        double qValueRemAlpha =
            expectedQValuesGivenBounds[indexBestAction] - contribSuQAlpha;
        double & vpiSuccessor = successorVPIs[su.su_state];
        actionIndex = -1;
//                                                                                dprint(PrSuGivenAlpha, contribSuQAlpha, lowBound, upBound);
        for (mlcore::Action* action : problem_->actions()) {
            if (!problem_->applicable(s, action))
                continue;
            actionIndex++;
            if (action == bestAction)   // VPI will be 0 for the best action
                continue;
            double PrSuGivenAction = statesProbs[actionIndex][su.su_state];
            double contribSuQAction =
                statesContribQValues[actionIndex][su.su_state];
            double qValueRemAction =
                expectedQValuesGivenBounds[actionIndex] - contribSuQAction;
            double vpiSuAction = computeVPI(PrSuGivenAlpha, qValueRemAlpha,
                                            PrSuGivenAction, qValueRemAction,
                                            lowBound, upBound);
//                                                                                dprint("   ", PrSuGivenAction, contribSuQAction, vpiSuAction);
            assert(vpiSuAction >= 0);
            assert(qValueRemAction < 2 * mdplib::dead_end_cost);
            assert(qValueRemAlpha < 2 * mdplib::dead_end_cost);
            vpiSuccessor = std::max(vpiSuccessor, vpiSuAction);
        }
//                                                                                dprint(vpiSuccessor, "********");
                                                                                if (show) {
                                                                                    dprint("  vpi for", su.su_state, vpiSuccessor);
                                                                                }
        totalVPI += vpiSuccessor;
    }

    if (totalVPI < mdplib::epsilon) {
//                                                                                if (upperBounds_[s] - s->cost() > 10) {
//                                                                                    mdplib_debug = true;
//                                                                                    {
//                                                                                        actionIndex = -1;
//                                                                                        dprint("vpi computation for", s, "bestAction", sampledAction, "bounds", s->cost(), upperBounds_[s]);
//                                                                                        for (mlcore::Action* action : problem_->actions()) {
//                                                                                            if (!problem_->applicable(s, action))
//                                                                                                continue;
//                                                                                            actionIndex++;
//                                                                                            dprint("    checking successors for ", action, "E[Qa|bounds]", expectedQValuesGivenBounds[actionIndex]);
//                                                                                            double totalP = 0.0;
//                                                                                            double EQcalc = problem_->cost(s, action);
//                                                                                            for (auto stateProb : statesProbs[actionIndex]) {
//                                                                                                totalP += stateProb.second;
//                                                                                                EQcalc += statesContribQValues[actionIndex][stateProb.first];
//                                                                                                dprint("          ", stateProb.first, "p(o|a)", stateProb.second,
//                                                                                                       "lb", stateProb.first->cost(), "ub", upperBounds_[stateProb.first],
//                                                                                                       "contrib", statesContribQValues[actionIndex][stateProb.first]);
//                                                                                            }
//                                                                                            assert(mdplib_math::equal(totalP, 1.0));
//                                                                                            assert(mdplib_math::equal(EQcalc, expectedQValuesGivenBounds[actionIndex]));
//                                                                                        }
//                                                                                    }
//                                                                                    dprint("***** vpi zero! ", s, s->cost(), upperBounds_[s], problem_->transition(s, bestAction).size());
//                                                                                    mdplib_debug = show;
//                                                                                }
                                                                                if (show) mdplib_debug = false;
        double pick = dis(gen);
        if (pick < alpha_)
            return sampleBiasedBounds(s, sampledAction);
        return nullptr;
    }
    double pick = dis(gen);
    double acc = 0.0;
    for (const mlcore::Successor& su : problem_->transition(s, sampledAction)) {
        acc += successorVPIs[su.su_state] / totalVPI;
        if (acc >= pick) {
            return su.su_state;
        }
    }
    assert(false);
    return nullptr;
}

double VPIRTDPSolver::computeVPI(double PrSuGivenAlpha, double qValueRemAlpha,
                                 double PrSuGivenAction, double qValueRemAction,
                                 double lowBound, double upBound) {
    double deltaPr = PrSuGivenAlpha - PrSuGivenAction;
    double deltaRemQVal = qValueRemAlpha - qValueRemAction;

    /*if (mdplib_math::equal(PrSuGivenAlpha, PrSuGivenAction)) {
        return 0.0;
    }
    double z = -deltaRemQVal / deltaPr;
    double q_lta_star_minus_q_lta = deltaRemQVal + lowBound * deltaPr;
    double q_hta_star_minus_q_hta = deltaRemQVal + upBound * deltaPr;

    if (z <= lowBound || z >= upBound) {
        double q_mta_star_minus_q_mta = deltaRemQVal + ((lowBound + upBound) / 2) * deltaPr;
        if (q_mta_star_minus_q_mta > 0.0) {
            return (q_lta_star_minus_q_lta + q_hta_star_minus_q_hta) / 2;
        } else {
            return 0.0;
        }
    }
    if (q_lta_star_minus_q_lta > 0.0)
        return q_lta_star_minus_q_lta * (z - lowBound) / (upBound - lowBound);
    else
        return q_hta_star_minus_q_hta * (upBound - z) / (upBound - lowBound);*/

    double normConst = upBound == lowBound ? 1 : (upBound - lowBound);
    double z = -deltaRemQVal / deltaPr;
    double vpi = -1;
    if (mdplib_math::equal(PrSuGivenAlpha, PrSuGivenAction)) {
        return std::max(0.0, qValueRemAlpha - qValueRemAction);
    } else if (mdplib_math::greaterThan(PrSuGivenAlpha, PrSuGivenAction)) {
        z = std::max(std::min(z , upBound), lowBound);
        vpi = (upBound - z) * (deltaRemQVal + deltaPr * (z + upBound) / 2);
    } else {
        z = std::min(std::max(z, lowBound), upBound);
        vpi = (z - lowBound) * (deltaRemQVal + deltaPr * (lowBound + z) / 2);
    }
    vpi /= normConst;
                                                                                /*if (vpi < 0) {
                                                                                    mpdlib_debug = true;
                                                                                    dprint(PrSuGivenAlpha, qValueRemAlpha, PrSuGivenAction, qValueRemAction, z, lowBound, upBound);
                                                                                    mpdlib_debug = false;
                                                                                }*/
    return vpi;
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
