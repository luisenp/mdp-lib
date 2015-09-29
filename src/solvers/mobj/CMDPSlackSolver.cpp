#include "../../../include/solvers/mobj/CMDPSlackSolver.h"

namespace mdplib_mobj_solvers
{

mlcore::Action* CMDPSlackSolver::solve(mlcore::State* s0)
{
    std::vector<int> constIndices;
    std::vector<double> constTargets;
    internalSolver_ = new CMDPSolver(problem_, 0, constIndices, constTargets);
    for (int i = 0; i < problem_->size(); i++) {
        double value = internalSolver_->solve(problem_->initialState());
        constIndices.push_back(i);
        if (slack_[i] == 0.0)
            break;
        constTargets.push_back(value + slack_[i] + 1.0e-5);
        internalSolver_->constIndices(constIndices);
        internalSolver_->constTargets(constTargets);
        internalSolver_->indexObjFun(i + 1);
    }
    policy_ = internalSolver_->policy();
}

} // mlsolvers
