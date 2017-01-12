#ifndef MDPLIB_RACETRACKPROBLEM_H
#define MDPLIB_RACETRACKPROBLEM_H

#include <unordered_map>
#include <vector>
#include <list>

#include "../../Action.h"
#include "../../Problem.h"
#include "../../State.h"

#include "RacetrackAction.h"
#include "RacetrackState.h"

namespace rtrack
{
    const char wall = 'X';
    const char goal = 'G';
    const char start = 'S';
    const char blank = ' ';
    const char error = 'o';
    const char pothole = 'P';

    /* Checks if the character is one of the valid track characters */
    inline bool checkValid(char c)
    {
        char valid[] = {wall, goal, start, blank, error, pothole};
        for (int i = 0; i < 6; i++)
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

    /* If true, the flat transition function will be used */
    bool useFlatTransition_ = false;

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

    /*
     * A flat transition function where every action has the same number
     * of successors in all states.
     */
    virtual std::list<mlcore::Successor> flatTransition(mlcore::State* s,
                                                        mlcore::Action* a);

public:

    /**
     * Constructs a racetrack problem instance from the given track file.
     */
    RacetrackProblem(const char* filename);

    virtual ~RacetrackProblem() {}

    std::vector<std::vector <char> > & track() { return track_; }

    void track(const std::vector<std::vector <char> > value) { track_ = value; }

    void mds(const int value) { mds_ = value; }

    int mds() const { return mds_; }

    void pError(const double value) { pError_ = value; }

    double pError() const { return pError_; }

    void pSlip(const double value) { pSlip_ = value; }

    double pSlip() const { return pSlip_; }

    mlcore::State* absorbing() const { return absorbing_; }

    void starts(const IntPairSet theStarts) { starts_ = theStarts; }

    void useFlatTransition(bool value) { useFlatTransition_ = value; }

    IntPairSet& starts() { return starts_; }

    /*
     * Returns the number of successors of the action, under the flat
     * transition, which is the same for every state but the initial state.
     */
    int numSuccessorsAction(RacetrackAction* a);

    /**
     * Sets the goal locations.
     */
    void goals(const IntPairSet theGoals) { goals_ = theGoals; }

    /**
     * Prints the track.
     */
    void printTrack(std::ostream& os);

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
