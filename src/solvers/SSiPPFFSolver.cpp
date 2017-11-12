#include "../include/solvers/SSiPPFFSolver.h"

namespace mlsolvers
{
    mlcore::Action* SSiPPFFSolver::solve(mlcore::State* s0)
    {
                                                                                dprint("foo");
        if (s0->bestAction() == nullptr) {
            // No action computed by SSiPP or FF-Replan. Must plan
            // This solver alternates between using SSiPP and using FF-Replan
            if (justUsedSSiPP_) {
                                                                                dprint("calling ff-replan");
                ffreplan_->solve(s0);
//                                                                                dprint("used ffreplan", s0->bestAction());
                justUsedSSiPP_ = false;
            } else {
                                                                                dprint("calling ssipp");
                ssipp_->solve(s0);
//                                                                                dprint("used ssipp", s0->bestAction());
                justUsedSSiPP_ = true;
            }
        }
                                                                                dprint("bar", (void *) s0->bestAction());
        return s0->bestAction();
    }

}
