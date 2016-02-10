#ifndef MDPLIB_RACETRACKPROBLEM_H
#define MDPLIB_RACETRACKPROBLEM_H

#include <unordered_map>
#include <vector>
#include <list>

#include "../../Action.h"
#include "../../Problem.h"
#include "../../State.h"

#include "RacetrackState.h"

namespace rtrack
{
	const char wall = 'X';
	const char goal = 'G';
	const char start = 'S';
	const char blank = ' ';
	const char error = 'o';

    /* Checks if the character is one of the valid track characters */
    inline bool checkValid(char c)
    {
        char valid[] = {wall, goal, start, blank, error};
        for (int i = 0; i < 5; i++)
            if (c == valid[i])
                return true;
        return false;
    }
}

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
 *   - Crashing with a wall does not return the car to the start but leaves
 *     it in the wall with speed 0. Moving out of a wall has a cost of 10
 *     actions. The only actions available at walls are the ones that attempt
 *     to bring the car back to the track.
 */
class RacetrackProblem : public mlcore::Problem
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

    /*
     * Returns the resulting state of applying the given acceleration to
     * the given state
     */
    RacetrackState* resultingState(RacetrackState* rts, int ax, int ay);

public:

    /**
     * Constructs a racetrack problem instance from the given track file.
     */
    RacetrackProblem(char* filename);

    virtual ~RacetrackProblem() {}

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
     * Overrides method from Problem.
     */
    virtual bool goal(mlcore::State* s) const;

    /**
     * Overrides method from Problem.
     */
    virtual std::list<mlcore::Successor> transition(mlcore::State* s,
                                                    mlcore::Action* a);

    /**
     * Overrides method from Problem.
     */
    virtual double cost(mlcore::State* s, mlcore::Action* a) const;

    /**
     * Overrides method from Problem.
     */
    virtual bool applicable(mlcore::State* s, mlcore::Action* a) const;
};

#endif // MDPLIB_RACETRACKPROBLEM_H
