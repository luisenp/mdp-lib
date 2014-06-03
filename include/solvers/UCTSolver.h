#ifndef MDPLIB_UCTSOLVER_H
#define MDPLIB_UCTSOLVER_H

#include <unordered_map>

#include "../problem.h"
#include "../state.h"
#include "../action.h"

typedef std::unordered_map<State*, ActionIntMap, StateHash, StateEqual> StateActionIntMap;
typedef std::unordered_map<State*, ActionDoubleMap, StateHash, StateEqual> StateActionDoubleMap;

/**
 * A SSSP solver using the UCT algorithm.
 *
 * See http://link.springer.com/chapter/10.1007/11871842_29
 *
 * Implementation based on pseudocode given by
 * http://www.morganclaypool.com/doi/pdf/10.2200/S00426ED1V01Y201206AIM017
 *
 */
class UCTSolver
{
private:
    Problem* problem_;
    double C_;

    StateSet visited_;
    StateIntMap counterS_;
    StateActionIntMap counterSA_;
    StateActionDoubleMap qvalues_;

    Action* pickUCB1Action(State* s);
    Action* pickAction(State* s, double C);

public:
    UCTSolver();

    virtual ~UCTSolver() {}

    /**
     * Creates an UCT solver for the given problem using the given exploration
     * parameter.
     *
     * @param problem The problem to be solved.
     * @param C The value of the exploration parameter.
     */
    UCTSolver(Problem* problem, double C);

    /**
     * Picks an action for the given state using the UCT algorithm.
     *
     * @param s0 The state for which the action will be chosen.
     * @param maxRollouts The maximum number of sample trajectories to gather.
     * @param cutoff The maximum depth allowed for each rollout.
     *
     * @return The action chosen by UCT.
     */
    Action* solve(State* s0, int maxRollouts, int cutoff);
};

#endif // MDPLIB_UCTSOLVER_H
