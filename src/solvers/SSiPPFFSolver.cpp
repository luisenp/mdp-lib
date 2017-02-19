#include "../include/solvers/SSiPPFFSolver.h"

namespace mlsolvers
{
    mlcore::Action* SSiPPFFSolver::solve(mlcore::State* s0)
    {
                                                                                dprint1("foo");
        if (s0->bestAction() == nullptr) {
            // No action computed by SSiPP or FF-Replan. Must plan
            // This solver alternates between using SSiPP and using FF-Replan
            if (justUsedSSiPP_) {
                                                                                dprint1("calling ff-replan");
                ffreplan_->solve(s0);
//                                                                                dprint2("used ffreplan", s0->bestAction());
                justUsedSSiPP_ = false;
            } else {
                                                                                dprint1("calling ssipp");
                ssipp_->solve(s0);
//                                                                                dprint2("used ssipp", s0->bestAction());
                justUsedSSiPP_ = true;
            }
        }
                                                                                dprint2("bar", (void *) s0->bestAction());
        return s0->bestAction();
    }

}
