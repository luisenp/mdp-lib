#ifndef MDPLIB_CONCURRENTSOLVER_H
#define MDPLIB_CONCURRENTSOLVER_H

#include <thread>
#include <mutex>

#include "solver.h"
#include "../state.h"
#include "../action.h"

namespace mlsolvers
{

/**
 * A ConcurrentSolver runs a specific base solver (e.g., LAO*, LRTDP) on a separate
 * thread. The base idea is to use the solver concurrently with the execution thread
 * to take advantage of the time it takes to execute and action for planning.
 *
 * The base planners supported are LAO* and LRTDP.
 */
class ConcurrentSolver
{
private:
    Solver& solver_;

    std::thread* solverThread;

    mutable std::mutex solverThreadMutex_;

    static void threadEntry(ConcurrentSolver* instance, mlcore::State* s0);

    void runSolver(mlcore::State* s0) const;

    bool keepRunning_ = true;

public:
    /**
     * Constructs a ConcurrentSolver that uses the given base MDP solver.
     *
     * @param the base MDP solver to use.
     */
    ConcurrentSolver(Solver& solver) : solver_(solver) { }

    /**
     * Destroys the ConcurrentSolver and the internat thread that runs the base
     * solver.
     */
    virtual ~ConcurrentSolver()
    {
        if (solverThread->joinable())
            solverThread->join();
        delete solverThread;
    }

    bool setKeepRunning(bool keepRunning)  { keepRunning_ = keepRunning; }

    /**
     *
     Returns the mutex used to lock the thread that runs the base solver.
     *
     * @return the mutex used to lock the thread that runs the base solver.
     */
    std::mutex& solverThreadMutex() { return solverThreadMutex_; }

    /**
     * Starts running the base solver on the given initial state.
     *
     * @param the initial state for which a solution policy is to be found.
     */
    void run(mlcore::State* s0);

    /**
     * Returns a *copy* of the best action found so far for the given state.
     *
     * It is recommended that the returned action is deleted right after using it
     * to reduce memory consumption.
     *
     * @param the state for which the action is to be returned.
     * @return the best action found so far for the given state.
     */
    mlcore::Action* getBestAction(mlcore::State* s);
};

}
#endif // MDPLIB_CONCURRENTSOLVER_H
