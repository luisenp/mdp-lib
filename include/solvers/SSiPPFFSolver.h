#ifndef MDPLIB_SSIPPFFSOLVER_H
#define MDPLIB_SSIPPFFSOLVER_H

#include "../ppddl/PPDDLProblem.h"

#include "FFUtil.h"
#include "FFReplanSolver.h"
#include "SSiPPSolver.h"
#include "Solver.h"


namespace mlsolvers
{

/**
 * Implements the FF-Replan solver as described in
 * http://repository.cmu.edu/cgi/viewcontent.cgi?article=3803&context=compsci
 */
class SSiPPFFSolver : public Solver
{

private:
    /* The problem to solve. */
    mlppddl::PPDDLProblem* problem_;

    /* An FF-Replan solver. */
    FFReplanSolver* ffreplan_;

    /* An SSiPP solver. */
    SSiPPSolver* ssipp_;

    /* A flag to indicate that the last internal planner used was SSiPP. */
    bool justUsedSSiPP_ = false;

public:

    /**
     * Creates an SSiPP-FF solver.
     * The following parameters are passed to FF:
     *   ffExecFilename, determinizedDomainFilename, templateProblemFilename
     *
     * The following parameters are passed to SSiPP:
     *   t, epsilon
     *
     * This constructor creates internal FF-Replan and SSiPP solvers. See
     * their class description for an explanation of the parameters.
     *
     * IMPORTANT: Make sure flagNewRound() is called every time a new round
     * using this solver is going to be performed.
     */
    SSiPPFFSolver(mlppddl::PPDDLProblem* problem,
                   std::string ffExecFilename,
                   std::string determinizedDomainFilename,
                   std::string templateProblemFilename,
                   int t,
                   double epsilon,
                   time_t maxPlanningTime = 3600)
    {
        ffreplan_ = new FFReplanSolver(problem,
                                       ffExecFilename,
                                       determinizedDomainFilename,
                                       templateProblemFilename,
                                       maxPlanningTime);
        ssipp_ = new SSiPPSolver(problem, epsilon, t);
    }

    virtual ~SSiPPFFSolver()
    {
        delete ffreplan_;
        delete ssipp_;
    }

    /** Necessary to avoid using FF at the beginning of a round. */
    void flagNewRound() { justUsedSSiPP_ = false; }

    /** Sets the maximum planning time allowed to the algorithm (in seconds). */
    virtual void maxPlanningTime(time_t theTime) {
        ffreplan_->maxPlanningTime(theTime);
        ssipp_->maxPlanningTime(theTime);
    }

    /**
     * Finds the best action for the given state. If the state is a
     * dead-end according to the given determinization, the method
     * returns a nullptr.
     */
    virtual mlcore::Action* solve(mlcore::State* s0);

};


}


#endif // MDPLIB_SSIPPFFSOLVER_H
