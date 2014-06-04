#include <vector>
#include <cmath>

#include "../../include/solvers/solver.h"
#include "../../include/solvers/UCTSolver.h"

UCTSolver::UCTSolver(Problem* problem, double C)
{
    problem_ = problem;
    C_ = C;
}

Action* UCTSolver::pickUCB1Action(State* s)
{
    return pickAction(s, C_);
}

Action* UCTSolver::pickAction(State* s, double C)
{
    double best = mdplib::dead_end_cost + 1;
    Action* bestAction = nullptr;
    for (std::pair<Action*,double> entry : qvalues_[s]) {
        Action* a = entry.first;
        if (counterSA_[s][a] == 0)
            return a;
        C = qvalues_[s][a];
        double tmp = entry.second - C*std::sqrt(std::log(counterS_[s]) / counterSA_[s][a]);
        if (tmp > mdplib::dead_end_cost)
            tmp = mdplib::dead_end_cost;
        if (tmp < best) {
            bestAction = a;
        }
    }
    return bestAction;
}

Action* UCTSolver::solve(State* s0, int maxRollouts, int cutoff)
{
    for (int r = 0; r < maxRollouts; r++) {
        State* tmp = s0;
        std::vector<int> cumCost(cutoff + 1);
        std::vector<State*> statesRoll(cutoff + 1);
        std::vector<Action*> actionsRoll(cutoff + 1);
        int maxSteps = 0;
        for (int i = 1; i <= cutoff; i++) {
            if (visited_.find(tmp) == visited_.end()) {
                counterS_[tmp] = 0;
                for (Action* a : problem_->actions()) {
                    if (!problem_->applicable(tmp, a))
                        continue;
                    counterSA_[tmp][a] = 0;
//                    qvalues_[tmp][a] = 0.0;
                    qvalues_[tmp][a] = qvalue(problem_, tmp, a).value();
                }
            }
            maxSteps = i;
            Action* a = pickUCB1Action(tmp);
            State* next = randomSuccessor(problem_, tmp, a);
            cumCost[i] = cumCost[i - 1] + problem_->cost(tmp, a).value();
            statesRoll[i] = tmp;
            actionsRoll[i] = a;
            tmp = next;
            if (problem_->goal(tmp))
                break;
        }

        for (int i = 1; i <= maxSteps; i++) {
            State* s = statesRoll[i];
            Action* a = actionsRoll[i];
            double newq = counterSA_[s][a]*qvalues_[s][a] + cumCost[maxSteps] - cumCost[i - 1];
            newq /= (counterSA_[s][a] + 1);
            qvalues_[s][a] = newq;
        }
    }
    return pickAction(s0, 0.0);
}
