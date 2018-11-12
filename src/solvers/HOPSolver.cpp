#include "../../include/solvers/HOPSolver.h"

#include "../../include/solvers/DeterministicSolver.h"


namespace mlsolvers
{

HOPSolver::HOPSolver(
    mlcore::Problem* problem, int maxSamples, int horizon, int maxTime) :
        problem_(problem),
        maxSamples_(maxSamples),
        horizon_(horizon),
        maxTime_(maxTime)
{ }


bool HOPSolver::ranOutOfTime() {
    if (maxTime_ > -1) {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto timeElapsed = std::chrono::duration_cast<
            std::chrono::milliseconds>(endTime - beginTime_).count();
        if (timeElapsed > maxTime_)
            return true;
    }
    return false;
}


mlcore::Action* HOPSolver::solve(mlcore::State* s0) {
                                                                                dprint("planning for",  s0);
    DeterministicSolver detSolver(problem_, det_random, problem_->heuristic());
    double bestQValue = mdplib::dead_end_cost + 1;
    mlcore::Action* bestAction = nullptr;
    for (mlcore::Action* action : problem_->actions()) {
        if (!problem_->applicable(s0, action))
            continue;
                                                                                dprint("  checking action",  action);
        double qValue = 0.0;
        for (auto& successor : problem_->transition(s0, action)) {
                                                                                dprint("      checking successor",  successor.su_state);
            double VSuccEst = 0.0;
            for (int i = 0; i < maxSamples_; i++) {
                                                                                std::cerr << i << " ";
                detSolver.solveTree(successor.su_state, horizon_);
                                                                                std::cerr << ", ";
                VSuccEst += detSolver.costLastPathFound()[successor.su_state];
            }
                                                                                std::cerr << std::endl;
            VSuccEst /= maxSamples_;
            qValue += successor.su_prob * VSuccEst;
        }
        qValue= (qValue * problem_->gamma()) + problem_->cost(s0, action);
        if (qValue <= bestQValue) {
            bestQValue = qValue;
            bestAction = action;
        }
    }
    return bestAction;
}

}
