#ifndef MDPLIB_CMDPLINPROGSOLVER_H
#define MDPLIB_CMDPLINPROGSOLVER_H

#include "../state.h"
#include "../lexi/mobj_problem.h"
#include "../lib/gurobi_c++.h"

#include <vector>

namespace mlsolvers
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
class CMDPLinProgSolver
{
private:
    mlmobj::MOProblem* problem_;

    mlcore::StateIntMap stateIndex_;

    std::vector<double> constTargets_;

    std::vector< std::vector<double> > MDPConstraints_;

    std::vector<double> MDPConstRS_;

    std::vector< std::vector<double> > CMDPConst_;

    std::vector<double> CMDPConstRS_;

public:
    CMDPLinProgSolver(mlmobj::MOProblem* problem, std::vector<double>& constTargets)
    {
        problem_ = problem;
        constTargets_ = constTargets;
    }

    virtual ~CMDPLinProgSolver() {}

    /**
     * Solves the associated problem using an LP-solver. The first cost function is
     * assumed to be the objective, and the rest are assumed to be constraints.
     *
     * @param s0 the state to start the search at.
     * @param constTargets the upper bounds for the constraints 2,3,...k.
     */
    void solve(mlcore::State* s0);
};

} // namespace mlsolvers

#endif // MDPLIB_CMDPLINPROGSOLVER_H
