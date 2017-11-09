#include "../../include/solvers/BoundedRTDPSolver.h"

namespace mlsolvers
{

BoundedRTDPSolver::BoundedRTDPSolver(mlcore::Problem* problem,
                         int maxTrials,
                         double epsilon)
    : problem_(problem), maxTrials_(maxTrials), epsilon_(epsilon) { }


void BoundedRTDPSolver::trial(mlcore::State* s) {
    mlcore::State* tmp = s;
    std::list<mlcore::State*> visited;
    while (!tmp->checkBits(mdplib::SOLVED)) {
        if (problem_->goal(tmp))
            break;

        visited.push_front(tmp);

        this->bellmanUpdate(tmp);

        if (tmp->deadEnd())
            break;

        tmp = randomSuccessor(problem_, tmp, tmp->bestAction());
    }
}

void BoundedRTDPSolver::initializeUpperBound(mlcore::State* s) {
    upperBounds_[s] = 100.0;
}

double BoundedRTDPSolver::bellmanUpdate(mlcore::State* s) {
    double bestQ = problem_->goal(s) ? 0.0 : mdplib::dead_end_cost;
    double bestUpperBound = problem_->goal(s) ? 0.0 : mdplib::dead_end_cost;
    bool hasAction = false;
    mlcore::Action* bestAction = nullptr;
    for (mlcore::Action* a : problem_->actions()) {
        if (!problem_->applicable(s, a))
            continue;
        hasAction = true;
        double qAction = 0.0;
        double upperBoundAction = 0.0;
        for (const mlcore::Successor& su : problem_->transition(s, a)) {
            qAction += su.su_prob * su.su_state->cost();
            if (upperBounds_.count(s) == 0)
                initializeUpperBound(s);
            upperBoundAction += su.su_prob * upperBounds_[s];
        }
        qAction = (qAction * problem_->gamma()) + problem_->cost(s, a);
        qAction = std::min(mdplib::dead_end_cost, qAction);
        upperBoundAction =
            (upperBoundAction * problem_->gamma()) + problem_->cost(s, a);
        upperBoundAction = std::min(mdplib::dead_end_cost,
                                    upperBoundAction);
        if (qAction <= bestQ) {
            bestQ = qAction;
            bestAction = a;
        }
        bestUpperBound = std::min(bestUpperBound, upperBoundAction);
    }
    if (!hasAction && bestQ >= mdplib::dead_end_cost)
        s->markDeadEnd();
    double residual = s->cost() - bestQ;
    s->setCost(bestQ);
    s->setBestAction(bestAction);
                                                                                assert(s->cost() < upperBounds_[s]);
    return fabs(residual);
}

mlcore::Action* BoundedRTDPSolver::solve(mlcore::State* s0) {
    int trials = 0;
    while (trials++ < maxTrials_) {
        trial(s0);
        if (upperBounds_[s0] - s0->cost() < epsilon_)
            break;
    }
}

}

