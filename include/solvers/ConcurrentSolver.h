#ifndef MDPLIB_CONCURRENTSOLVER_H
#define MDPLIB_CONCURRENTSOLVER_H

#include <thread>
#include <mutex>

#include "../Action.h"
#include "../State.h"

#include "Solver.h"

namespace mlsolvers
{

/**
 * A ConcurrentSolver runs a specific base solver (e.g., LAO*, LRTDP)
 * on a separate thread. The base idea is to use the solver concurrently with
 * the execution thread to take advantage of the time it takes to execute and
 * action for planning.
 * The base solvers supported are LAO* and LRTDP.
 *
 * The class provides a mutex object to ensure that no race conditions arise
 * between the planning and execution threads try to access state variables
 * (e.g., cost,  best actions).
 */
class ConcurrentSolver
{
private:
    Solver& solver_;

    mlcore::State* state_ = nullptr;

    std::thread* solverThread;

    static void threadEntry(ConcurrentSolver* instance);

    void runSolver() const;

    bool keepRunning_ = true;

public:
    /**
     * Constructs a ConcurrentSolver that uses the given base MDP solver.
     *
     * @param solver the base MDP solver to use.
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

    /**
     * Sets the state the base solver must plan for in the next iteration.
     *
     * @param state the state that the base solver must plan for.
     */
    void setState(mlcore::State* state) { state_ = state; }

    /**
     * Sets whether the base solver should keep running or not.
     *
     * @param keepRunning true if the base solver should keep running,
     *                    false otherwise.
     */
    void setKeepRunning(bool keepRunning)  { keepRunning_ = keepRunning; }

    /**
     * Starts running the base solver to get an action for the state stored.
     */
    void run();

};

}
#endif // MDPLIB_CONCURRENTSOLVER_H
