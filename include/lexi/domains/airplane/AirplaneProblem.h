#ifndef MDPLIB_AIRPLANEPROBLEM_H
#define MDPLIB_AIRPLANEPROBLEM_H

#include <vector>

#include "../state.h"
#include "../action.h"

#include "../../lexi_problem.h"

namespace mllexi
{

#define AIRPLANE_WINDY 1
#define AIRPLANE_CALM 0

#define INSIDE_AIRPLANE -1

class AirplaneProblem : public LexiProblem
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

    using LexiProblem::goal;

    AirplaneProblem(int travelGoal,
                    std::vector< std::vector<double> > distances,
                    std::vector< double > windProbs,
                    std::vector< int > initialLocations);

    virtual ~AirplaneProblem() {}

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
    virtual mlcore::SuccessorsList transition(mlcore::State* s, mlcore::Action* a, int index);

    /**
     * Overrides method from LexiProblem.
     */
    virtual bool goal(mlcore::State* s, int index) const;
};

}

#endif // MDPLIB_AIRPLANEPROBLEM_H
