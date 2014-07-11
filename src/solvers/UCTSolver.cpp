#include <vector>
#include <cmath>

#include "../../include/solvers/solver.h"
#include "../../include/solvers/UCTSolver.h"

namespace mlsolvers
{
    UCTSolver::UCTSolver(mlcore::Problem* problem, double C, int maxRollouts, int cutoff)
    {
        problem_ = problem;
        C_ = C;
        maxRollouts_ = maxRollouts;
        cutoff_ = cutoff;
    }

    mlcore::Action* UCTSolver::pickUCB1Action(mlcore::State* s)
    {
        return pickAction(s, C_);
    }

    mlcore::Action* UCTSolver::pickAction(mlcore::State* s, double C)
    {
        double best = mdplib::dead_end_cost + 1;
        mlcore::Action* bestAction = nullptr;
        for (std::pair<mlcore::Action*,double> entry : qvalues_[s]) {
            mlcore::Action* a = entry.first;
            if (counterSA_[s][a] == 0)
                return a;
            double ucb1 = ucb1Cost(s, a, entry.second);
            if (ucb1 < best) {
                bestAction = a;
                best = ucb1;
            }
        }
        return bestAction;
    }

    double UCTSolver::ucb1Cost(mlcore::State* s, mlcore::Action* a, double C)
    {
        double cost = qvalues_[s][a] - C*std::sqrt(2 * std::log(counterS_[s]) / counterSA_[s][a]);
        if (cost > mdplib::dead_end_cost)
            cost = mdplib::dead_end_cost;
        return cost;
    }

    mlcore::Action* UCTSolver::solve(mlcore::State* s0)
    {
        for (int r = 0; r < maxRollouts_; r++) {
            mlcore::State* tmp = s0;
            std::vector<int> cumCost(cutoff_ + 1);
            std::vector<mlcore::State*> statesRoll(cutoff_ + 1);
            std::vector<mlcore::Action*> actionsRoll(cutoff_ + 1);
            int maxSteps = 0;
            for (int i = 1; i <= cutoff_; i++) {
                if (visited_.insert(tmp).second) {
                    counterS_[tmp] = 0;
                    for (mlcore::Action* a : problem_->actions()) {
                        if (!problem_->applicable(tmp, a))
                            continue;
                        counterSA_[tmp][a] = 0;
                        qvalues_[tmp][a] = qvalue(problem_, tmp, a);
                    }
                }
                maxSteps = i;
                mlcore::Action* a = pickUCB1Action(tmp);
                mlcore::State* next = randomSuccessor(problem_, tmp, a);
                cumCost[i] = cumCost[i - 1] + problem_->cost(tmp, a);
                statesRoll[i] = tmp;
                actionsRoll[i] = a;
                tmp = next;
                if (problem_->goal(tmp))
                    break;
            }

            for (int i = 1; i <= maxSteps; i++) {
                mlcore::State* s = statesRoll[i];
                mlcore::Action* a = actionsRoll[i];
                double newq = counterSA_[s][a]*qvalues_[s][a] + cumCost[maxSteps] - cumCost[i - 1];
                newq /= (counterSA_[s][a] + 1);
                qvalues_[s][a] = newq;
                counterS_[s]++;
                counterSA_[s][a]++;
            }
        }
        return pickAction(s0, 0.0);
    }
}

