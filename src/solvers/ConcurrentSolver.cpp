#include "../../include/solvers/ConcurrentSolver.h"

namespace mlsolvers
{

    void ConcurrentSolver::threadEntry(ConcurrentSolver* instance, mlcore::State* s0)
    {
        instance->runSolver(s0);
    }

    void ConcurrentSolver::runSolver(mlcore::State* s0) const
    {
        while (keepRunning_) {
            solverThreadMutex_.lock();
            solver_.solve(s0);
            solverThreadMutex_.unlock();
        }
    }

    void ConcurrentSolver::run(mlcore::State* s0)
    {
        solverThread = new std::thread(&threadEntry, this, s0);
    }

    mlcore::Action* ConcurrentSolver::getBestAction(mlcore::State* s)
    {
        solverThreadMutex_.lock();
        mlcore::Action* action = (mlcore::Action*) malloc(sizeof(s->bestAction()));
        memcpy(action, s->bestAction(), sizeof(s->bestAction()));
        solverThreadMutex_.unlock();
        return action;
    }
}
