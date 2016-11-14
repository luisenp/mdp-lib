#ifndef MDPLIB_RFFSOLVER_H
#define MDPLIB_RFFSOLVER_H


#include "../ppddl/PPDDLProblem.h"

#include "FFUtil.h"
#include "Solver.h"


namespace mlsolvers
{

/**
 * Implements the RFF solver as described in
 * http://ifaamas.org/Proceedings/aamas2010/pdf/01%20Full%20Papers/25_04_FP_0529.pdf
 */
class RFFSolver : public Solver
{
private:
    /* The problem to solve. */
    mlppddl::PPDDLProblem* problem_;

    /* The file where FF is to be called at. */
    std::string ffExecFilename_;

    /* The file name where the determinized PPDDL domain is defined. */
    std::string determinizedDomainFilename_;

    /*
     * The file name with the template used to create the problems to be solved
     * by FF.
     */
    std::string templateProblemFilename_;

    /* The file name of the updated problems solved by FF. */
    const std::string currentProblemFilename_ = "/tmp/rff_tmpfile";

    /* The total time available for planning (in seconds). */
    int maxPlanningTime_;

    /* The time at which the solve method was last called. */
    time_t startingPlanningTime_;

    /* The initial atoms that are removed during the PPDDL parsing. */
    std::string removedInitAtoms_;

    ////////////////////////////////////////////////////////////////////////////
    //                               FUNCTIONS                                //
    ////////////////////////////////////////////////////////////////////////////

public:

    RFFSolver(mlppddl::PPDDLProblem* problem,
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

    virtual ~RFFSolver() {}

    /**
     * Finds the best action for the given state. If the state is a
     * dead-end according to the given determinization, the method
     * returns a nullptr.
     */
    virtual mlcore::Action* solve(mlcore::State* s0);
};

}

#endif // MDPLIB_RFFSOLVER_H
