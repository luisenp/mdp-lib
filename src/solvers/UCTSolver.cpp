#include <vector>
#include <cmath>

#include "../../include/solvers/Solver.h"
#include "../../include/solvers/UCTSolver.h"

namespace mlsolvers
{

mlcore::Action* UCTSolver::pickUCB1Action(UCTNode* node)
{
    return pickAction(node, C_);
}


mlcore::Action* UCTSolver::pickAction(UCTNode* node, double C)
{
    double best = mdplib::dead_end_cost + 1;
    mlcore::Action* bestAction = nullptr;
    for (std::pair<mlcore::Action*, double> entry : qvalues_[node]) {
        mlcore::Action* a = entry.first;
        if (counters_node_action_[node][a] == 0)   // unexplored action
            return a;
        double ucb1 = ucb1Cost(node, a, entry.second);
                                                                                if (C_ != -1.0)
                                                                                    dprint1("error");
        double C = C_ == -1.0 ? entry.second : C_;
        if (ucb1 < best) {
            bestAction = a;
            best = ucb1;
        }
    }
    return bestAction;
}


double UCTSolver::ucb1Cost(UCTNode* node, mlcore::Action* a, double C)
{
    if (use_qvalues_for_c_)
        C = qvalues_[node][a];
    double cost = qvalues_[node][a] - C
        * std::sqrt(2 * std::log(counters_node_[node])
                    / counters_node_action_[node][a]);
    if (cost > mdplib::dead_end_cost)
        cost = mdplib::dead_end_cost;
    return cost;
}


mlcore::Action* UCTSolver::solve(mlcore::State* s0)
{
                                                                                dprint1("solving");
    UCTNode* root = new UCTNode(s0, 0);
    for (int r = 0; r < maxRollouts_; r++) {
        UCTNode* tmp_node = root;
        std::vector<int> cumCost(cutoff_ + 1);
        std::vector<UCTNode*> nodesRoll(cutoff_ + 1);
        std::vector<mlcore::Action*> actionsRoll(cutoff_ + 1);
        int maxSteps = 0;
        for (int i = 1; i <= cutoff_; i++) {
            bool first_time_seen = visited_.insert(tmp_node).second;
                                                                                dprint3("  ", i, tmp_node->state_);
            if (first_time_seen) {
                counters_node_[tmp_node] = 0;
                                                                                dprint1("     expanding");
                for (mlcore::Action* a : problem_->actions()) {
                    if (!problem_->applicable(tmp_node->state_, a))
                        continue;
                    counters_node_action_[tmp_node][a] = 0;
                    qvalues_[tmp_node][a] =
                        qvalue(problem_, tmp_node->state_, a);
                }
                                                                                dprint1("     expanded");
            }
                                                                                dprint1("  was-created");
            if (problem_->goal(tmp_node->state_))
                break;
            maxSteps = i;
            mlcore::Action* a = pickUCB1Action(tmp_node);
                                                                                if (a == nullptr) {
                                                                                    dprint3(" found null for", tmp_node->state_, problem_->goal(tmp_node->state_));
                                                                                }
            mlcore::State* next =
                randomSuccessor(problem_, tmp_node->state_, a);
            cumCost[i] = cumCost[i - 1] + problem_->cost(tmp_node->state_, a);
            nodesRoll[i] = tmp_node;
            actionsRoll[i] = a;
            tmp_node = new UCTNode(next, tmp_node->depth_ + 1);
        }
                                                                                dprint1("rollout-ended");

        for (int i = 1; i <= maxSteps; i++) {
            dprint2("  ", i);
            UCTNode* node = nodesRoll[i];
                                                                                dprint2("  got node ", node->state_);
            mlcore::Action* a = actionsRoll[i];
                                                                                dprint2("  got action", (void *) a);
            double newq = counters_node_action_[node][a] * qvalues_[node][a] +
                cumCost[maxSteps] - cumCost[i - 1];
                                                                                dprint1("  new-q step-1");
            newq /= (counters_node_action_[node][a] + 1);
                                                                                dprint1("  new-q computed");
            qvalues_[node][a] = newq;
            counters_node_[node]++;
            counters_node_action_[node][a]++;
        }
                                                                                dprint1("updates ended");
    }
                                                                                dprint1("done");
    return pickAction(root, 0.0);
}

}

