#ifndef MDPLIB_FFREPLAN_H
#define MDPLIB_FFREPLAN_H

#include "../ppddl/PPDDLProblem.h"

#include "FFUtil.h"
#include "Solver.h"


namespace mlsolvers
{

/**
 * Implements the FF-Replan solver as described in
 * https://vvvvw.aaai.org/Papers/ICAPS/2007/ICAPS07-045.pdf
 */
class FFReplanSolver : public Solver
{

private:
    /* The problem to solve. */
    mlppddl::PPDDLProblem* problem_;

    /* The file where FF is to be called at. */
    std::string ffExecFilename_;

    /* The file name where the determinized PPDDL domain is defined. */
    std::string determinizedDomainFilename_;

    /*
     * The file name of the template used to create the problems to be solved
     * by FF.
     */
    std::string templateProblemFilename_;

    /* The file name of the updated problems solved by FF. */
    const std::string currentProblemFilename_ = "/tmp/ff-replan_tmpfile";

    /* The total time available for planning (in seconds). */
    int maxPlanningTime_;

    /* The time at which the solve method was last called. */
    time_t startingPlanningTime_;

    /* The initial atoms that are removed during the PPDDL parsing. */
    std::string removedInitAtoms_;

    /*
     * Calls FF on the deterministic version of the problem to find a plan
     * for state s. The output parameter fullPlan stores the complete plan for
     * the determinized domain, starting from state s.
     * The function returns the first action in the plan.
     */
    mlcore::Action*
    callFF(mlcore::State* s, std::vector<std::string>& fullPlan) const;

public:

    FFReplanSolver(mlppddl::PPDDLProblem* problem,
                   std::string ffExecFilename,
                   std::string determinizedDomainFilename,
                   std::string templateProblemFilename,
                   time_t maxPlanningTime = 3600) :
        problem_(problem),
        ffExecFilename_(ffExecFilename),
        determinizedDomainFilename_(determinizedDomainFilename),
        templateProblemFilename_(templateProblemFilename),
        maxPlanningTime_(maxPlanningTime)

    {
        removedInitAtoms_ =
            storeRemovedInitAtoms(templateProblemFilename_, problem);
    }

    virtual ~FFReplanSolver() {}

    /** Sets the maximum planning time allowed to the algorithm (in seconds). */
    virtual void maxPlanningTime(time_t theTime) { maxPlanningTime_ = theTime; }

    /**
     * Finds the best action for the given state. If the state is a
     * dead-end according to the given determinization, the method
     * returns a nullptr.
     */
    virtual mlcore::Action* solve(mlcore::State* s0);

};


}

#endif // MDPLIB_FFREPLAN_H
