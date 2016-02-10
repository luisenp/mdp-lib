#ifndef CMDPLAGRANGESEARCHSOLVER_H
#define CMDPLAGRANGESEARCHSOLVER_H

#include "../Solver.h"
#include "../../Problem.h"
#include "../../State.h"
#include "../../../include/solvers/VISolver.h"

#include "MObjSolvers.h"

namespace mdplib_mobj_solvers
{

class CMDPLagrangeSearchSolver : public mlsolvers::Solver
{
private:
    mlmobj::MOProblem* problem_;

    mlsolvers::Solver* solver_;

    std::vector<double> getExpectedCostOfGreedyPolicy(mlcore::State* s0,
                                                      int numIterations);

    std::vector<double> setNewWeightsForProblem(
        std::vector<int> comb, std::vector<double> WeightValues);

    double evaluateObjectiveFunction(
        std::vector<double> lambdas, mlcore::State* s0, mlcore::Action** bestAction);
public:
    /**
     * Creates a CMDPLagrangeSearchSolver for the specified CMDP problem.
     *
     * @param problem The CMDP problem to be solved.
     */
    CMDPLagrangeSearchSolver(mlmobj::MOProblem* problem)
    {
        problem_ = problem;
        solver_ = new mlsolvers::VISolver(problem_, 1000000, 1.0e-10);
    }

    ~CMDPLagrangeSearchSolver() { delete solver_; }

    /**
     * Solves the associated problem.
     * TODO: update documentation.
     */
    virtual mlcore::Action* solve(mlcore::State* s0);
};

}
#endif // CMDPLAGRANGESEARCHSOLVER_H
