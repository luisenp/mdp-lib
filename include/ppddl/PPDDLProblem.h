#ifndef MDPLIB_PPDDLPROBLEM_H
#define MDPLIB_PPDDLPROBLEM_H

#include "mini-gpt/problems.h"
#include "mini-gpt/rational.h"

#include "../Problem.h"

namespace mlppddl
{

typedef std::pair<state_t *, Rational> successor_t;

/**
 * A class representing a PPDDL problem. The implementation is based on
 * the mini-gpt library (see http://ldc.usb.ve/~bonet/reports/JAIR-mgpt.pdf).
 */
class PPDDLProblem : public mlcore::Problem
{
private:
    problem_t* pProblem_;

    successor_t display_[DISP_SIZE];

public:
    PPDDLProblem(problem_t* pProblem);

    virtual ~PPDDLProblem()
    {
        problem_t::unregister_use(pProblem_);
    }

    problem_t* pProblem()  { return pProblem_; }

    /**
     * Returns the action with the given name.
     */
    mlcore::Action* getActionFromName(std::string actionName);

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

}

#endif // MDPLIB_PPDDLPROBLEM_H
