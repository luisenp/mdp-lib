#ifndef MDPLIB_CMDPLINPROGSOLVER_H
#define MDPLIB_CMDPLINPROGSOLVER_H

#include <cassert>
#include <vector>

#include "../../lib/gurobi_c++.h"

#include "../../mobj/mobj_problem.h"
#include "../../state.h"

#include "../RandomPolicy.h"

namespace mdplib_mobj_solvers
{

/**
 * Solves a constrained MDP using an LP-formulation.
 * The result is a randomized policy.
 *
 * See Dolgov and Durfee, 2005.
 * http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.104.4661&rep=rep1&type=pdf
 *
 * This class uses the Gurobi LP-Solver available at http://www.gurobi.com/
 */
class CMDPSolver
{
private:
    mlmobj::MOProblem* problem_;

    mlsolvers::RandomPolicy* policy_;

    mlcore::StateIntMap stateIndex_;

    int indexObjFun_;

    std::vector<int> constIndices_;

    std::vector<double> constTargets_;

    std::vector< std::vector<double> > MDPConstraints_;

    std::vector<double> MDPConstRS_;

    std::vector< std::vector<double> > CMDPConst_;

    std::vector<double> CMDPConstRS_;

    double solvePrimal(mlcore::State* s0);

    double solveDual(mlcore::State* s0);

    void createVariablesAndObjFunForPrimal(int numVariables,
                                           int numActions,
                                           GRBModel& model,
                                           GRBVar* variables,
                                           GRBLinExpr &objFun);

    void createLPConstraintsLHSForPrimal(GRBVar* variables,
                                         int numActions,
                                         GRBLinExpr* MDPConstLHS,
                                         GRBLinExpr* CMDPConstLHS);

    void ComputePolicyFromPrimalSolution(GRBVar* variables,
                                         int numStates,
                                         int numActions);

public:
    /**
     * Creates a CMDPSolver for the given problem. This constructor receives the
     * index of the objective function and a permutation of the remaining
     * indices corresponding to the CMDP constraints.
     *
     * Let <C[0], C[1], ..., C[m]> be the vector of cost functions defined
     * in the given MOProblem. Then, the constructed CMDPSolver solves:
     *
     * min C[indexObjFun_]
     * s.t.
     *      C[constIndices[i]] <= constTargets[i], for i = 1:m-1
     */
    CMDPSolver(mlmobj::MOProblem* problem,
               int indexObjFun,
               std::vector<int>& constIndices,
               std::vector<double>& constTargets)
    {
        assert(constTargets.size() == constIndices.size());
        assert(constTargets.size() < problem->size());
        problem_ = problem;
        constTargets_ = constTargets;
        constIndices_ = constIndices;
        indexObjFun_ = indexObjFun;
        policy_ = nullptr;
    }

    virtual ~CMDPSolver()
    {
        delete policy_;
    }

    void indexObjFun(int idx) { indexObjFun_ = idx; }

    void constTargets(std::vector<double>& ct) { constTargets_ = ct; }

    void constIndices(std::vector<int>& ci) { constIndices_ = ci; }

    /**
     * Solves the associated problem using an LP-solver.
     * The first cost function is assumed to be the objective,
     * and the rest are assumed to be constraints.
     * The function returns the value of the optimal policy.
     *
     * @param s0 The state to start the search at.
     * @param constTargets The upper bounds for the constraints 2,3,...k.
     *
     * @return The value of the optimal policy.
     */
    double solve(mlcore::State* s0);

    /**
     * Returns the optimal random policy for the problem.
     */
    mlsolvers::RandomPolicy* policy() { return policy_; }
};

} // namespace mlsolvers

#endif // MDPLIB_CMDPLINPROGSOLVER_H
