#ifndef MDPLIB_LEXIRACETRACKPROBLEM_H
#define MDPLIB_LEXIRACETRACKPROBLEM_H

#include <unordered_map>
#include <vector>
#include <list>

#include "../lexi_problem.h"
#include "../lexi_state.h"
#include "../../action.h"

#include "LexiRacetrackState.h"

namespace mllexi
{

/**
 * A class implementing the Racetrack problem.
 *
 * See: A. Barto, S.J. Bradtke and S.P. Singh,
 *      "Learning to act using real-time dynamic programming."
 *      Artificial Intelligence 72.1 (1995); 81-138
 *
 * This implementation has a few modifications with respect to the original
 * problem:
 *
 *   - There is a maximum deterministic speed, below which all actions are
 *     deterministic.
 *
 *   - There are error-prone locations where there is a non-zero probability
 *     of choosing the wrong action.
 *     (see http://anytime.cs.umass.edu/shlomo/papers/PZicaps14.pdf).
 *
 *   - Crashing with a wall does not return the car to the start but leaves it in
 *     the wall with speed 0. Moving out of a wall has a cost of 10 actions.
 *     The only actions available at walls are the ones that attempt to bring the
 *     car back to the track.
 */
class LexiRacetrackProblem : public LexiProblem
{
private:
    mlcore::State* absorbing_;

    /* Maximum deterministic speed */
    int mds_ = 2;

    /* Probability of slipping as in the original racetrack */
    double pSlip_ = 0.10;

    /* Probability of picking wrong action */
    double pError_ = 0.05;

    /* Stores the track to be used in this problem */
    std::vector<std::vector <char> > track_;

    /* All the start locations */
    IntPairSet starts_;

    /* All the goal locations */
    IntPairSet goals_;

    /* Returns the resulting state of applying the given acceleration to the given state */
    LexiRacetrackState* resultingState(LexiRacetrackState* rts, int ax, int ay);

public:

    using LexiProblem::goal;

    /**
     * Constructs a racetrack problem instance from the given track file.
     */
    LexiRacetrackProblem(char* filename, int size);

    virtual ~LexiRacetrackProblem() {}

    /**
     * Returns the track.
     */
    std::vector<std::vector <char> > & track() { return track_; }

    /**
     * Sets the track.
     */
    void track(std::vector<std::vector <char> > theTrack) { track_ = theTrack; }

    /**
     * Sets the start locations.
     */
    void starts(IntPairSet theStarts) { starts_ = theStarts; }

    /**
     * Sets the goal locations.
     */
    void goals(IntPairSet theGoals) { goals_ = theGoals; }

    /**
     * Prints the track.
     */
    void printTrack(std::ostream& os);

    /**
     * Sets the maximum deterministic speed for this problem.
     */
    void setMDS(int mds) { mds_ = mds; }

    /**
     * Sets the probability of chossing an unintended acceleration.
     */
    void setPError(double pError) { pError_ = pError; }

    /**
     * Sets the probability of slipping.
     */
    void setPSlip(double pSlip) { pSlip_ = pSlip; }

    /**
     * Overrides method from LexiProblem.
     */
    virtual bool applicable(mlcore::State* s, mlcore::Action* a) const;

    /**
     * Overrides method from LexiProblem.
     */
    virtual double cost(mlcore::State* s, mlcore::Action* a, int i) const;

    /**
     * Overrides method from LexiProblem.
     */
    virtual std::list<mlcore::Successor>
    transition(mlcore::State* s, mlcore::Action* a, int index);

    /**
     * Overrides method from LexiProblem.
     */
    virtual bool goal(mlcore::State* s, int index) const;

};


}

#endif // MDPLIB_LEXIRACETRACKPROBLEM_H
