#include "../../include/solvers/ConcurrentSolver.h"

namespace mlsolvers
{

    void ConcurrentSolver::threadEntry(ConcurrentSolver* instance, mlcore::State* s0)
    {
        instance->runSolver(s0);
    }

    void ConcurrentSolver::runSolver(mlcore::State* s0) const
    {
        solver_.solve(s0);
    }

    void ConcurrentSolver::run(mlcore::State* s0)
    {
        solverThread = new std::thread(&threadEntry, this, s0);
        solverThread->join();
    }

}
