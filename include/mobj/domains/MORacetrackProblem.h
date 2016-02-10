#ifndef MDPLIB_MORACETRACKPROBLEM_H
#define MDPLIB_MORACETRACKPROBLEM_H

#include <unordered_map>
#include <vector>
#include <list>

#include "../../Action.h"

#include "../MObjProblem.h"
#include "../MObjState.h"

#include "MORacetrackState.h"

namespace mlmobj
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
class MORacetrackProblem : public MOProblem
{
private:
    mlcore::State* absorbing_;

    /* Maximum deterministic speed */
    int mds_ = 2;

    /* Use the safety cost function for the second level */
    bool useSafety_ = false;

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

    static const char unsafeChar_ = '.';

    /* Returns the resulting state of applying the given acceleration to the given state */
    MORacetrackState* resultingState(MORacetrackState* rts, int ax, int ay);

    /* Checks if the given location is safe (not a wall or unsafe street) */
    bool isSafeLocation(int x, int y);

    /* Adds successors for (source, idAction), according to the transition function for
     * the safety variable.  The method assumes that dest->safe() reflects the safety
     * of its location. */
    void addSuccessors(int idAction, MORacetrackState* source,
                        MORacetrackState* dest, double prob);

    bool dummyApplicable(mlcore::State* s, mlcore::Action* a) const;

public:

    using MOProblem::goal;

    /**
     * Constructs a racetrack problem instance from the given track file.
     */
    MORacetrackProblem(char* filename, int size);

    virtual ~MORacetrackProblem() {}

    /**
     * Specifies whether the second objective function is safety or not.
     */
    void useSafety(bool value) { useSafety_ = value; }

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
     * Overrides method from MOProblem.
     */
    virtual bool applicable(mlcore::State* s, mlcore::Action* a) const;

    /**
     * Overrides method from MOProblem.
     */
    virtual double cost(mlcore::State* s, mlcore::Action* a, int i) const;

    /**
     * Overrides method from MOProblem.
     */
    virtual mlcore::SuccessorsList transition(mlcore::State* s, mlcore::Action* a, int index);

    /**
     * Overrides method from MOProblem.
     */
    virtual bool goal(mlcore::State* s, int index) const;

};


}

#endif // MDPLIB_MORACETRACKPROBLEM_H
