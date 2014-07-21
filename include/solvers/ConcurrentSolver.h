#ifndef MDPLIB_CONCURRENTSOLVER_H
#define MDPLIB_CONCURRENTSOLVER_H

#include <thread>

#include "solver.h"
#include "../state.h"

namespace mlsolvers
{

class ConcurrentSolver
{
private:
    Solver& solver_;

    std::thread* solverThread;

    static void threadEntry(ConcurrentSolver* instance, mlcore::State* s0);

    void runSolver(mlcore::State* s0) const;

public:
    /**
     * Constructs a ConcurrentSolver that uses the given base MDP solver.
     *
     * @param the base MDP solver to use.
     */
    ConcurrentSolver(Solver& solver) : solver_(solver) { }

    virtual ~ConcurrentSolver()
    {
        if (solverThread->joinable())
            solverThread->join();
        delete solverThread;
    }

    /**
     * Starts running the base solver on the given initial state.
     *
     * @param the initial state for which a solution policy is to be found.
     */
    void run(mlcore::State* s0);
};

}
#endif // MDPLIB_CONCURRENTSOLVER_H
