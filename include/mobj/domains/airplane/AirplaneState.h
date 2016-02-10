#ifndef MDPLIB_AIRPLANESTATE_H
#define MDPLIB_AIRPLANESTATE_H

#include <vector>

#include "../State.h"

#include "../../MObjState.h"

namespace mlmobj
{

class AirplaneState : public MOState
{

friend class AirplaneProblem;
friend class AirplaneHeuristic;

private:

    int airplaneLocation_;

    std::vector<int> personLocations_;

    std::vector<int> windConditions_;

    virtual std::ostream& print(std::ostream& os) const;

public:
    AirplaneState(AirplaneState* state);

    AirplaneState(int airplaneLocation,
                  std::vector<int> personLocations_,
                  std::vector<int> windConditions_,
                  MOProblem* problem_);

    virtual ~AirplaneState() {}

    /**
     * Overrides method from State.
     */
    virtual mlcore::State& operator=(const mlcore::State& rhs);

    /**
     * Overrides method from State.
     */
    virtual bool operator==(const mlcore::State& rhs) const;

    /**
     * Overrides method from State.
     */
    virtual bool equals(mlcore::State* other) const;

    /**
     * Overrides method from State.
     */
    virtual int hashValue() const;
};


}
#endif // MDPLIB_AIRPLANESTATE_H
