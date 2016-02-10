#ifndef MDPLIB_AIRPLANEPROBLEM_H
#define MDPLIB_AIRPLANEPROBLEM_H

#include <vector>

#include "../State.h"
#include "../Action.h"

#include "../../MObjProblem.h"

namespace mlmobj
{

#define AIRPLANE_WINDY 1
#define AIRPLANE_CALM 0

#define INSIDE_AIRPLANE -1

class AirplaneProblem : public MOProblem
{

friend class AirplaneHeuristic;

private:

    mlcore::State* absorbing_;

    int travelGoal_;

    std::vector< std::vector<double> > distances_;

    std::vector<double> windProbs_;

    std::vector<int> initialLocations_;

    double minDistance_ = mdplib::dead_end_cost + 1;

public:

    using MOProblem::goal;

    AirplaneProblem(int travelGoal,
                    std::vector< std::vector<double> > distances,
                    std::vector< double > windProbs,
                    std::vector< int > initialLocations);

    virtual ~AirplaneProblem() {}

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

#endif // MDPLIB_AIRPLANEPROBLEM_H
