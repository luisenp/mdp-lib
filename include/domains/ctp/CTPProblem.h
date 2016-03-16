#ifndef MDPLIB_CTPPROBLEM_H
#define MDPLIB_CTPPROBLEM_H

#include "../../domains/ctp/CTPState.h"
#include "../../util/graph.h"

#include "../../Problem.h"

namespace ctp
{
    const int BLOCKED = 0;
    const int OPEN = 1;
}

/**
 * A class implementing the Canadian Traveler Problem.
 * See - https://www.cs.ucsb.edu/~suri/cs235/Rlist/noMapNavigation.pdf
 */
class CTPProblem : public mlcore::Problem
{
private:
    int goal_;
    int start_;
    Graph* roads_;
    std::vector< std::vector <double> > probs_;
    CTPState* absorbing_;

    void init();

public:
    /**
     * Constructs a Canadian Traveler problem instances with the given roads,
     * bad weather probabilities, start state and goal.
     */
    CTPProblem(Graph* roads,
               std::vector< std::vector <double> >& probs,
               int start,
               int goal);

    ~CTPProblem()
    {
        delete roads_;
    }

    /**
     * Reads a CTP problem from a file and constructs a problem instance
     * representing this problem.
     *
     * @param filename the name of the file describing the problem.
     */
    CTPProblem(const char* filename);

    /**
     * Returns a graph structure containing all roads in the problem and their
     * associated costs.
     */
    Graph* roads()
    {
        return roads_;
    }

    /**
     * Returns the goal location for this problem.
     */
    int goalLocation()
    {
        return goal_;
    }

    /**
     * Overrides method from Problem.
     */
    virtual bool goal(mlcore::State* s) const;

    /**
     * Overrides method from Problem.
     */
    virtual std::list<mlcore::Successor>
    transition(mlcore::State* s, mlcore::Action* a);

    /**
     * Overrides method from Problem.
     */
    virtual double cost(mlcore::State* s, mlcore::Action* a) const;

    /**
     * Overrides method from Problem.
     */
    virtual bool applicable(mlcore::State* s, mlcore::Action* a) const;
};

#endif // MDPLIB_CTPPROBLEM_H
