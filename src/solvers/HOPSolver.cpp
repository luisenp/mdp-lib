#include "../../include/solvers/HOPSolver.h"

#include "../../include/solvers/DeterministicSolver.h"


namespace mlsolvers
{

HOPSolver::HOPSolver(mlcore::Problem* problem, int maxSamples, int maxTime) :
    problem_(problem),
    maxSamples_(maxSamples),
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
    DeterministicSolver detSolver(problem_, det_random, problem_->heuristic());
    double bestQValue = mdplib::dead_end_cost + 1;
    mlcore::Action* bestAction = nullptr;
    for (mlcore::Action* action : problem_->actions()) {
        if (!problem_->applicable(s0, action))
            continue;
        double qValue = 0.0;
        for (auto& successor : problem_->transition(s0, action)) {
            double VSuccEst = 0.0;
            for (int i = 0; i < maxSamples_; i++) {
                detSolver.solve(successor.su_state);
                VSuccEst += detSolver.costLastPathFound();
            }
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
