#include "../../include/solvers/ConcurrentSolver.h"

namespace mlsolvers
{

    void ConcurrentSolver::threadEntry(ConcurrentSolver* instance)
    {
        instance->runSolver();
    }

    void ConcurrentSolver::runSolver() const
    {
        while (keepRunning_) {
            solver_.solve(state_);
            //dprint1("Finished Trial");
        }
    }

    void ConcurrentSolver::run()
    {
        solverThread = new std::thread(&threadEntry, this);
    }

}
