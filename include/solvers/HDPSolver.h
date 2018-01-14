#ifndef MDPLIB_HDPSOLVER_H
#define MDPLIB_HDPSOLVER_H

#include <climits>
#include <list>
#include <vector>

#include "../Problem.h"
#include "../State.h"

#include "Solver.h"

namespace mlsolvers
{

/**
 * An implementation of the HDP and HPD(i,j) algorithms as described
 * in http://web.cs.ucla.edu/pub/stat_ser/R317.pdf (IJCAI'03).
 */
class HDPSolver : public Solver
{
private:
    /* The problem to solve. */
    mlcore::Problem* problem_;

    /* The tolerance for the residual error. */
    double epsilon_;

    /*
     * The minimum plausibilitiy (note that a plausibility of 0 is greater than
     * a plausibility of 1, 1 greater than 2, and so on.
     */
    int minPlaus_;

    /* A vector to hold the kappa transition plausibilities for successors. */
    std::vector<int> kappaList_;

    /* Performs a DFS while simultaneously labeling states as solved. */
    bool dfs(mlcore::State* s, double plaus);

    /* An index used to compute the connected component algorithm. */
    int index_;

    /* A stack of states used to compute the connected components. */
    std::list<mlcore::State*> stateStack_;

    /* A map from state to indices used to compute the connected components. */
    mlcore::StateIntMap indices_;

    /* A map from state to indices used to compute the connected components. */
    mlcore::StateIntMap low_;

    /* The set of states in the connected components stack. */
    mlcore::StateSet inStack_;

    /* Stores the states that were solved. */
    mlcore::StateSet solvedStates_;

    /* The maximum number of trials. */
    int maxTrials_;

    /* Maximum planning time in milliseconds. */
    int maxTime_;

    /* The time at which planning began. */
    std::chrono::time_point<std::chrono::high_resolution_clock> beginTime_;

public:
    HDPSolver(mlcore::Problem* problem,
              double epsilon = 1.0e-6,
              int minPlaus = INT_MAX) :
        problem_(problem),
        epsilon_(epsilon),
        minPlaus_(minPlaus),
        maxTrials_(1000000),
        maxTime_(-1)
    {
        kappaList_ = std::vector<int>(2048, 0);
    }

    /**
     * Computes the kappa function for the given probabilities.
     */
    int kappa(double prob, double maxProb);

    /**
     * Computes the transition plausibilities for the given successors.
     */
    void computeKappa(std::list<mlcore::Successor>& successors,
                       std::vector<int>& kappaList);

    /**
     * Removes labels on the all labeled states.
     */
    void clearLabels()
    {
        for (mlcore::State* state : solvedStates_)
            state->clearBits(mdplib::SOLVED);
        solvedStates_.clear();
    }

    virtual ~HDPSolver() { }

    /**
     * Solves the associated problem using the HDP algorithm.
     *
     * @param s0 The state to start the search at.
     */
    virtual mlcore::Action* solve(mlcore::State* s0);

    /**
     * Sets the maximum number of trials allowed to the algorithm.
     */
    virtual void maxTrials(time_t theTrials) { maxTrials_  = theTrials; }

    /**
     * Sets the maximum planning time allowed to the algorithm (milliseconds).
     */
    virtual void maxPlanningTime(time_t theTime) { maxTime_ = theTime; }
};

} // namespace mlsolvers

#endif // MDPLIB_HDPSOLVER_H
