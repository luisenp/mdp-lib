#ifndef MDPLIB_UCTSOLVER_H
#define MDPLIB_UCTSOLVER_H

#include <unordered_map>

#include "../problem.h"
#include "../state.h"
#include "../action.h"

namespace mlsolvers
{
    typedef std::unordered_map<mlcore::State*,
                                mlcore::ActionIntMap,
                                mlcore::StateHash,
                                mlcore::StateEqual> StateActionIntMap;

    typedef std::unordered_map<mlcore::State*,
                                mlcore::ActionDoubleMap,
                                mlcore::StateHash,
                                mlcore::StateEqual> StateActionDoubleMap;

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
        mlcore::Problem* problem_;
        double C_;

        mlcore::StateSet visited_;
        mlcore::StateIntMap counterS_;
        StateActionIntMap counterSA_;
        StateActionDoubleMap qvalues_;

        mlcore::Action* pickUCB1Action(mlcore::State* s);
        mlcore::Action* pickAction(mlcore::State* s, double C);

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
        UCTSolver(mlcore::Problem* problem, double C);

        /**
         * Returns the Q-values estimated by the UCT algorithm.
         */
        StateActionDoubleMap& qvalues() { return qvalues_; }

        /**
         * Returns the counter for state-action pair visits.
         */
        StateActionIntMap& counterSA() { return counterSA_; }

        /**
         * Computes the UCB1 cost of the given state-action pair with the current
         * visit counts.
         *
         * @param s The state for which the cost is going to be computed.
         * @param a The action for which the cost is going to be computed.
         * @param C The value of the exploration parameter to be used.
         *
         * @return The cost of the state-action according to the UCB1 formula.
         */
        double ucb1Cost(mlcore::State* s, mlcore::Action* a, double C);

        /**
         * Picks an action for the given state using the UCT algorithm.
         *
         * @param s0 The state for which the action will be chosen.
         * @param maxRollouts The maximum number of sample trajectories to gather.
         * @param cutoff The maximum depth allowed for each rollout.
         *
         * @return The action chosen by UCT.
         */
        mlcore::Action* solve(mlcore::State* s0, int maxRollouts, int cutoff);
    };

}


#endif // MDPLIB_UCTSOLVER_H
