#ifndef MDPLIB_SAILINGPROBLEM_H
#define MDPLIB_SAILINGPROBLEM_H

#include <vector>

#include "../../problem.h"
#include "../../state.h"
#include "../../action.h"

#include "SailingState.h"
#include "SailingAction.h"

enum tack_t {AWAY = 0, DOWN = 1, CROSS = 2, UP = 3, INTO = 4};


class SailingProblem : public mlcore::Problem
{
private:
    short startX_;
    short startY_;
    short goalX_;
    short goalY_;
    short rows_;
    short cols_;

    std::vector<double> costs_;

    int tack(SailingState* state, SailingAction* action) const;

    bool inLake(SailingState* state);

    mlcore::State* absorbing_;

public:
    SailingProblem(short startX, short startY,
                   short goalX, short goalY,
                   short rows, short cols,
                   std::vector<double> & costs);

    virtual ~SailingProblem() {}

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

#endif // MDPLIB_SAILINGPROBLEM_H
