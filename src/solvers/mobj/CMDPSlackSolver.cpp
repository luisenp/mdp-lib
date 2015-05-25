#include "../../../include/solvers/mobj/CMDPSlackSolver.h"

namespace mlsolvers
{

mlcore::Action* CMDPSlackSolver::solve(mlcore::State* s0)
{
    std::vector<int> constIndices;
    std::vector<double> constTargets;
    dprint1("here1");
    internalSolver_ = new CMDPSolver(problem_, 0, constIndices, constTargets);
    dprint1("here2 ");
    for (int i = 0; i < problem_->size(); i++) {
        std::cout << "Obj. Fun " << i << "\n s.t. " << std::endl;
        for (int j = 0; j < constIndices.size(); j++) {
            std::cout << "    C" << constIndices[j] << " <= " << constTargets[j] << std::endl;
        }
        double value = internalSolver_->solve(problem_->initialState());
        constIndices.push_back(i);
        constTargets.push_back(value + slack_[i] + 1.0e-5);
        internalSolver_->constIndices(constIndices);
        internalSolver_->constTargets(constTargets);
        internalSolver_->indexObjFun(i + 1);
        std::cout << "\n******************************************************** \n" << std::endl;
    }
    policy_ = internalSolver_->policy();
    policy_->print(std::cerr);
}

} // mlsolvers
