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
            solverThreadMutex_.lock();
            solver_.solve(state_);
            solverThreadMutex_.unlock();
        }
    }

    void ConcurrentSolver::run()
    {
        solverThread = new std::thread(&threadEntry, this);
    }

}
