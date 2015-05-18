#include "../../../include/solvers/mobj/CMDPSlackSolver.h"

namespace mlsolvers
{

mlcore::Action* CMDPSlackSolver::solve(mlcore::State* s0)
{
    std::vector<int> constIndices;
    std::vector<double> constTargets;
    internalSolver_ = new CMDPSolver(problem_, 0, constIndices, constTargets);
    for (int i = 0; i < problem_->size(); i++) {
        double value = internalSolver_->solve(problem_->initialState());
        dprint4("Value of C", i, " is ", value);
        constIndices.push_back(i);
        constTargets.push_back(value + slack_[i]);
        dprint2("Number of constraints ", constTargets.size());
        internalSolver_->constIndices(constIndices);
        internalSolver_->constTargets(constTargets);
        internalSolver_->indexObjFun(i + 1);
        std::cout << "\n ******************************************************** \n" << std::endl;
    }
    policy_ = internalSolver_->policy();
}

} // mlsolvers
