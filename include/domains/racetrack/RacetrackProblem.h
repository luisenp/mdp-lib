#ifndef MDPLIB_RACETRACKPROBLEM_H
#define MDPLIB_RACETRACKPROBLEM_H

#include <unordered_map>
#include <vector>
#include <list>

#include "../../problem.h"
#include "../../state.h"
#include "../../action.h"

namespace rtrack
{
	const char wall = 'X';
	const char goal = 'G';
	const char start = 'S';
	const char blank = ' ';
	const char error = 'o';
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
 */
class RacetrackProblem : public mlcore::Problem
{
private:
    /* Maximum deterministic speed */
    int mds_;

    /* Probability of slipping as in the original racetrack */
    double pSlip_;

    /* Probability of picking wrong action */
    double pError_;

    /* Stores the track to be used in this problem */
    std::vector<std::vector <char> > grid_;

    /* All the start locations */
    IntPairSet starts_;

    /* All the goal locations */
    IntPairSet goals_;

    mlcore::State* absorbing_;

public:

    /**
     * Constructs a racetrack problem instance from the given track file.
     */
    RacetrackProblem(char* filename);

    virtual ~RacetrackProblem() {}

    /**
     * Returns the character grid that stores the track.
     */
    std::vector<std::vector <char> > & grid() { return grid_; }

    /**
     * Sets the maximum deterministic speed for this problem.
     */
    void setMDS(int mds) { mds_ = mds; }

    /**
     * Overrides method from Problem.
     */
    virtual bool goal(mlcore::State* s) const;

    /**
     * Overrides method from Problem.
     */
    virtual std::list<mlcore::Successor> transition(mlcore::State* s, mlcore::Action* a);

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
