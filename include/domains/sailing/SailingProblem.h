#ifndef MDPLIB_SAILINGPROBLEM_H
#define MDPLIB_SAILINGPROBLEM_H

#include <vector>

#include "../../Action.h"
#include "../../Problem.h"
#include "../../State.h"

#include "SailingState.h"
#include "SailingAction.h"
#include "SailingNoWindHeuristic.h"

enum tack_t {AWAY = 0, DOWN = 1, CROSS = 2, UP = 3, INTO = 4};

class SailingNoWindHeuristic;
class SailingState;

class SailingProblem : public mlcore::Problem
{
friend SailingNoWindHeuristic;

private:
    short startX_;
    short startY_;
    short goalX_;
    short goalY_;
    short rows_;
    short cols_;

    std::vector<double> costs_;

    double* windTransition_;

    int tack(const SailingState* state, const SailingAction* action) const;

    bool inLake(const short x, const short y) const;

    mlcore::State* absorbing_;

    /*
     * If true, the transition funtion returns the same number of successors
     * for all states.
     */
    bool useFlatTransition_;

public:
    SailingProblem(short startX, short startY, short startWind,
                   short goalX, short goalY,
                   short rows, short cols,
                   std::vector<double> & costs,
                   double* windTransition,
                   bool useFlatTransition = false);

    virtual ~SailingProblem() {}

    /**
     * Overrides method from Problem.
     */
    virtual bool goal(mlcore::State* s) const;

    /**
     * Overrides method from Problem.
     */
    virtual std::list<mlcore::Successor>
    transition(mlcore::State* s, mlcore::Action* a);

    void useFlatTransition(bool value) { useFlatTransition_ = value; }

    /**
     * Overrides method from Problem.
     */
    virtual double cost(mlcore::State* s, mlcore::Action* a) const;

    /**
     * Overrides method from Problem.
     */
    virtual bool applicable(mlcore::State* s, mlcore::Action* a) const;
};

#endif // MDPLIB_SAILINGPROBLEM_H
